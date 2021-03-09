#include "facedb.h"
#include "resnetfacedescriptorcomputer.h"
#include "resnetfacedescriptormetric.h"
#include "openfacedescriptorcomputer.h"
#include "openfacedescriptormetric.h"
#include "labeldata.h"

#include <iostream>
#include <cassert>
#include <filesystem>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>



std::string getNameFromLabel(const std::string& label)
{
	static const auto labelMap = generateLabelMap();

	try
	{
		return labelMap.at(label);
	}
	catch (const std::out_of_range&)
	{
		return label;	// in case name lookup failed, simply return the label itself
	}
}	// getNameFromLabel

template <class DescriptorComputer>
void execute(DescriptorComputer&& descriptorComputer, const std::string& database, const std::string& cache, const std::string& query, double tolerance)
{
	FaceDb<DescriptorComputer> faceDb{ std::forward<DescriptorComputer>(descriptorComputer) };	
	faceDb.setReporter([](const std::string& message) { std::cout << message << std::endl; });	
	
	if (std::filesystem::is_directory(database))	// dataset directory specified
	{
		faceDb.create(database);
		
		if (!cache.empty())			// if the database cache file is specified, save descriptors there,
			faceDb.save(cache);		// so we don't have to recreate it every time
	}
	else	// load the database from the existing file
	{
		faceDb.load(database);
	}
	
	if (!query.empty())		// if query is specified, try to find this person in the database
	{
		cv::Mat im = cv::imread(query, cv::IMREAD_COLOR);

		auto drawText = [&im, thickness=1, padding=4](int bottom, const cv::String& text, cv::Scalar color, int fontFace, double fontScale)
		{
			int baseLine;
			cv::Size szText = cv::getTextSize(text, fontFace, fontScale, thickness, &baseLine);

			bottom -= baseLine + thickness + padding;	// adjust the bottom coordinate of the text for OpenCV
			cv::putText(im, text, cv::Point{ (im.cols - szText.width) / 2, bottom }, fontFace, fontScale, color, thickness, cv::LINE_AA);
			return bottom - szText.height;	// return the top coordinate of the text
		};

		int y = im.rows;	// the bottom coordinate of the text to draw
		auto [label, similarity] = faceDb.find(query);	// find the best match
		if (similarity <= tolerance)
		{
			y = drawText(y, std::to_string(similarity), cv::Scalar(0, 140, 255), cv::FONT_HERSHEY_COMPLEX_SMALL, 1);
			drawText(y, getNameFromLabel(label), cv::Scalar(139, 200, 0), cv::FONT_HERSHEY_COMPLEX, 1);
		}	// face identified
		else
		{
			drawText(y, "Unknown", cv::Scalar(0, 0, 255), cv::FONT_HERSHEY_COMPLEX, 1);
		}

		cv::imshow("Doppelganger", im);
		cv::waitKey();
	}	// not an empty query
}	// execute


void printUsage()
{
	std::cout << "Usage: doppelganger [-h]"
		" --database=<dataset directory or cached database file>"
		" [--cache=<cache file (output)>]"
		" [--query=<image file>]"
		" [--tolerance=<a positive float>]"
		" [--algorithm=<ResNet or OpenFace>]" << std::endl;
}	// printUsage


int main(int argc, char* argv[])
{
	try
	{

		static const cv::String keys =
			"{help h usage ?        |       | Print the help message  }"
			"{database              |<none> | The path to a dataset directory or a cached file of previously computed face descriptors }"
			"{cache                 |       | If not empty, specifies the output file path where face descriptors will be saved to }"
			"{query                 |       | If not empty, specifies the path to an image of a person that needs to be recognized }"
			"{tolerance             |0.7    | Defines the largest allowed difference between two faces considered the same (float) }"
			"{algorithm             |ResNet | Specifies face recognition algorithm to use (ResNet or OpenFace) }";
			
		cv::CommandLineParser parser(argc, argv, keys);
		parser.about("Doppelganger\n(c) Yaroslav Pugach");

		if (parser.has("help"))
		{
			printUsage();
			return 0;
		}

		std::string db = parser.get<std::string>("database");
		std::string cache = parser.get<std::string>("cache");
		std::string query = parser.get<std::string>("query");
		std::string algorithm = parser.get<std::string>("algorithm");
		double tolerance = parser.get<double>("tolerance");

		if (!parser.check())
		{
			parser.printErrors();
			printUsage();
			return -1;
		}

		
		std::transform(algorithm.cbegin(), algorithm.cend(), algorithm.begin(), static_cast<int (*)(int)>(&std::tolower));
		if (algorithm == "resnet")
		{			
			ResNetFaceDescriptorComputer descriptorComputer{ "./models/shape_predictor_5_face_landmarks.dat"
                                                            , "./models/dlib_face_recognition_resnet_model_v1.dat" };
			execute(std::move(descriptorComputer), db, cache, query, tolerance);
		}
		else if (algorithm == "openface")
		{
			// OpenFace suggests using outerEyesAndNose alignment:
			// https://cmusatyalab.github.io/openface/visualizations/#2-preprocess-the-raw-images
			OpenFaceDescriptorComputer<OpenFaceAlignment::OuterEyesAndNose> descriptorComputer{ "./models/shape_predictor_68_face_landmarks.dat"
                                                                                            ,  "./models/nn4.v2.t7" };
			execute(std::move(descriptorComputer), db, cache, query, tolerance);
		}
		else throw std::invalid_argument("Unsupported algorithm: " + algorithm);
	}	// try
	catch (const dlib::cuda_error& e)
	{
		std::cerr << e.what() << std::endl << "Try disabling CUDA by setting DLIB_USE_CUDA=OFF and rebuild the project." << std::endl;
		return -2;
	}
	catch (const std::bad_alloc& e)
	{
		std::cerr << e.what() << std::endl <<
			"It looks like there is not enough memory for the program. Try the following options:"
			"\n1) Close other software to save system resources and run again."
			"\n2) Disable concurrency by configuring the project with PARALLEL_EXECUTION=OFF, then rebuild."
			<< std::endl;
		return -3;
	}
	catch (const std::exception& e)		// all Dlib and OpenCV exceptions inherit from std::exception
	{
		std::cerr << e.what() << std::endl;
		return -4;
	}
	catch (...)
	{
		std::cerr << "Unknown exception occurred" << std::endl;
		return -5;
	}

	return 0;
}
