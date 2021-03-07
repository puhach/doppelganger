#include "facedb.h"
//#include "dnnfacedescriptorcomputer.h"
//#include "resnet.h"
#include "resnetfacedescriptorcomputer.h"
#include "resnetfacedescriptormetric.h"
#include "openfacedescriptorcomputer.h"
#include "openfacedescriptormetric.h"
#include "labeldata.h"

#include <iostream>
#include <cassert>
#include <chrono>	// TEST!
#include <filesystem>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

//using namespace std;
//using namespace cv;


// An auxiliary function for drawing facial landmarks
void drawLandmarks(cv::Mat& image, const std::vector<cv::Point>& landmarks)
{
	for (auto i = 0; i < landmarks.size(); ++i)
	{
		const auto& lm = landmarks.at(i);
		cv::Point center(lm.x, lm.y);
		cv::circle(image, center, 3, cv::Scalar(0, 255, 0), -1);
		cv::putText(image, std::to_string(i), center, cv::FONT_HERSHEY_COMPLEX_SMALL, 0.5, cv::Scalar(0, 0, 255), 1);
	}
}

// An auxiliary function for saving landmarks to a text file
void saveLandmarks(const std::string& fileName, const std::vector<cv::Point>& landmarks)
{
	std::ofstream ofs(fileName, std::ofstream::out);
	assert(ofs.good());

	ofs << landmarks.size() << std::endl;
	for (const auto lk : landmarks)
	{
		ofs << lk.x << " " << lk.y << std::endl;
	}

	assert(ofs.good());
}

// An auxiliary function for loading landmarks from a text file
std::vector<cv::Point> loadLandmarks(const std::string& fileName)
{
	std::ifstream ifs(fileName, std::ifstream::in);
	assert(ifs.good());

	size_t n;
	ifs >> n;

	std::vector<cv::Point> landmarks(n);
	for (size_t i = 0; i < n; ++i)
	{
		unsigned long x, y;
		ifs >> x >> y;

		//landmarks.emplace_back(x, y);
		landmarks[i].x = x;
		landmarks[i].y = y;
	}

	return landmarks;
}

std::string getNameFromLabel(const std::optional<std::string>& label)
{
	if (!label)
		return "Unknown";

	static const auto labelMap = generateLabelMap();

	try
	{
		return labelMap.at(*label);
	}
	catch (const std::out_of_range&)
	{
		return *label;	// in case name lookup failed, simply return the label itself
	}
}	// getNameFromLabel

template <class DescriptorComputer>
void execute(DescriptorComputer&& descriptorComputer, const std::string& database, const std::string& cache, const std::string& query, double tolerance)
{
	FaceDb<DescriptorComputer> faceDb{ std::forward<DescriptorComputer>(descriptorComputer) };	
	//auto lam = [](const DescriptorComputer::Descriptor&, const DescriptorComputer::Descriptor&) -> double { return 22; };
	//FaceDb<DescriptorComputer, decltype(lam)> faceDb( std::forward<DescriptorComputer>(descriptorComputer), lam );	// TEST!
	faceDb.setReporter([](const std::string& message) { std::cout << message << std::endl; });
	
	if (std::filesystem::is_directory(database))	// dataset directory specified
	{
		faceDb.create(database);
		//// TEST!
		//faceDb.save(cache);
		//faceDb.clear();
		//faceDb.load(cache);
		//faceDb.enroll("Z:/n00000102/n00000102_00000068.JPEG", "Bachchan");
		//faceDb.enroll("Z:/n00000102/n00000102_00000434.JPEG", "Bachchan");
		//faceDb.enroll("Z:/n00002238/n00002238_00000156.JPEG", "Selena");
		////faceDb.enroll("Z:/n00002238/n00002238_00000655.JPEG", "Selena");
		
		if (!cache.empty())			// if the database cache file is specified, save descriptors there,
			faceDb.save(cache);		// so we don't have to recreate it every time
	}
	else	// load the database from the existing file
	{
		faceDb.load(database);
	}
	
	if (!query.empty())		// if query is specified, try to find this person in the database
	{
		std::string name = getNameFromLabel(faceDb.find(query, tolerance));
		cv::Mat im = cv::imread(query, cv::IMREAD_COLOR);
		int baseLine;
		cv::Size szText = cv::getTextSize(name, cv::FONT_HERSHEY_COMPLEX, 1, 1, &baseLine);
		cv::putText(im, name, cv::Point{ (im.cols - szText.width) / 2, (im.rows + szText.height + baseLine) / 2 }
			, cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(0, 255, 0), 1, cv::LINE_AA);
		cv::imshow("Doppelganger", im);
		cv::waitKey();
		std::cout << name << std::endl;
	}
}	// execute

int main(int argc, char* argv[])
{
	using namespace std::string_literals;

	try
	{
		//const std::string landmarkDetectionModel5{ "./shape_predictor_5_face_landmarks.dat" };
		//const std::string landmarkDetectionModel68{ "./shape_predictor_68_face_landmarks.dat" };
		//const std::string resNetModel{ "./dlib_face_recognition_resnet_model_v1.dat" };
		//const std::string openFaceModel{ "./nn4.v2.t7" };

		// doppelganger --enroll=<image/directory> --find=<file>
		/// doppelganger --load=<descriptors file> [--find=<file>]
		/// doppelganger [--load=<descriptors file> ] [--enroll=<image/directory>] [--save=<descriptors file>] [--find=<file>]
		// doppelganger --dataset=<directory> --query=<image file>
		// doppelganger --cache=<descriptors file> --query=<image file>
		// doppelganger --database=<file or directory> [--cache=<file>] [--query=<image file>]

		std::string db = "./dataset";
		std::string cache = "./descriptors.db";
		std::string query = "./test/sofia-solares.jpg";
		//std::string query = "./test/shashikant-pedwal.jpg";		
		//std::string algorithm = "ResNet";
		std::string algorithm = "OpenFace";
		double tolerance = 0.7;
		
		//std::transform(algorithm.cbegin(), algorithm.cend(), algorithm.begin(), [](char c) { return std::tolower(c); });
		std::transform(algorithm.cbegin(), algorithm.cend(), algorithm.begin(), static_cast<int (*)(int)>(&std::tolower));

		if (algorithm == "resnet")
		{			
			ResNetFaceDescriptorComputer descriptorComputer{ "./shape_predictor_5_face_landmarks.dat", "./dlib_face_recognition_resnet_model_v1.dat" };
			execute(std::move(descriptorComputer), db, cache, query, tolerance);
		}
		else if (algorithm == "openface")
		{
			// OpenFace suggests using outerEyesAndNose alignment:
			// https://cmusatyalab.github.io/openface/visualizations/#2-preprocess-the-raw-images
			OpenFaceDescriptorComputer<OpenFaceAlignment::OuterEyesAndNose> descriptorComputer{ "./shape_predictor_68_face_landmarks.dat",  "./nn4.v2.t7" };
			execute(std::move(descriptorComputer), db, cache, query, tolerance);
		}
		else throw std::invalid_argument("Unsupported algorithm: " + algorithm);
	}
	catch (const dlib::cuda_error& e)
	{
		std::cerr << e.what() << std::endl << "Try disabling CUDA by setting DLIB_USE_CUDA=OFF and rebuild the project." << std::endl;
		return -1;
	}
	catch (const std::bad_alloc& e)
	{
		std::cerr << e.what() << std::endl <<
			"It looks like there is not enough memory for the program. Try the following options:"
			"\n1) Close other software to save system resources and run again."
			"\n2) Disable concurrency by configuring the project with PARALLEL_EXECUTION=OFF, then rebuild."
			<< std::endl;
		return -2;
	}
	catch (const std::exception& e)		// all Dlib and OpenCV exceptions inherit from std::exception
	{
		std::cerr << e.what() << std::endl;
		return -3;
	}
	catch (...)
	{
		std::cerr << "Unknown exception occurred" << std::endl;
		return -4;
	}

	return 0;
}
