#include "facedb.h"
//#include "dnnfacedescriptorcomputer.h"
//#include "resnet.h"
#include "resnetfacedescriptorcomputer.h"
#include "resnetfacedescriptormetric.h"


#include <iostream>
#include <cassert>
#include <chrono>	// TEST!

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



int main(int argc, char* argv[])
{
	

	try
	{
		// doppelganger --enroll=<image/directory> --find=<file>
		/// doppelganger --load=<descriptors file> [--find=<file>]
		/// doppelganger [--load=<descriptors file> ] [--enroll=<image/directory>] [--save=<descriptors file>] [--find=<file>]
		// doppelganger --dataset=<directory> --query=<image file>
		// doppelganger --cache=<descriptors file> --query=<image file>
		// doppelganger --database=<file or directory> [--cache=<file>] [--query=<image file>]

		std::string db = "./dataset";
		std::string cache = "./descriptors.csv";

		/*auto now = std::chrono::steady_clock::now();
		DnnFaceDescriptorComputer<ResNet> one("./shape_predictor_68_face_landmarks.dat", "./dlib_face_recognition_resnet_model_v1.dat");
		auto elapsed1 = std::chrono::steady_clock::now() - now;
		std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed1).count() << std::endl;
		int tmp = 33;
		now = std::chrono::steady_clock::now();
		DnnFaceDescriptorComputer<ResNet> two = one;
		auto elapsed2 = std::chrono::steady_clock::now() - now;
		std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed2).count() << std::endl;*/

		//FaceDb<ResNetFaceDescriptorComputer, ResNetFaceDescriptorComparator> faceDb(db, ResNetFaceDescriptorComparator(0.33));
		//faceDb.setFaceDescriptorComparator(ResNetFaceDescriptorComparator(0.7));
		// comparator may not necessarily be DnnFaceDescriptorComparator and be parametrized by DNN!
		//FaceDb<ResNetFaceDescriptorComputer, DnnFaceDescriptorComparator<ResNet>> faceDb(db, 0.33); 
		//FaceDb<ResNetFaceDescriptorComputer, FaceDescriptorComparator<ResNet::Output>> faceDb(db, 0.33);
		//FaceDb<ResNetFaceDescriptorComputer, ResNetFaceDescriptorMetric> faceDb(db);
		auto consoleReporter = [](const std::string& msg) {	std::cout << msg << std::endl; };
		FaceDb<ResNetFaceDescriptorComputer, L2Distance<ResNetFaceDescriptorComputer::Descriptor>> faceDb{ std::move(consoleReporter) };
		//FaceDb faceDb(db, cache);
		faceDb.create(db);
		//FaceDb<ResNetFaceDescriptorComputer> faceDb(db);	// TODO: do we need a default constructor?
		//faceDb.find("some file", ResNetDescriptorComparator(0.7));
		//faceDb.save("z:/akla/my.db");
		faceDb.load("z:/my.db");
		auto first = faceDb.find("./test/sofia-solares.jpg", 0.6);
		auto second = faceDb.find("./test/shashikant-pedwal.jpg", 0.6);
		std::cout << (first ? *first : "unknown") << " " << (second ? *second : "unknown") << std::endl;
		//FaceDb<ResNetFaceDescriptorComputer> faceDb("z:/aalal/my.db");
		//FaceDb<DnnFaceDescriptorComputer<ResNet>> faceDb(db);
		//faceDb.find("some file", DnnDescriptorComparator<ResNet>(0.7));
		//faceDb.find(query);
		//FaceDb disp;
		////disp.enroll("./dataset", "./descriptors.csv");
		//disp.load(db);
		//disp.save(cache);
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
	catch (const std::exception& e)		// TODO: handle dlib exceptions
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
