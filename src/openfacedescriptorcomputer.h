#ifndef OPENFACEDESCRIPTORCOMPUTER_H
#define OPENFACEDESCRIPTORCOMPUTER_H


#include "openface.h"
//#include "dnnfacedescriptorcomputer.h"
//#include "openfacedescriptor.h"
//#include "dlibfaceextractor.h"
#include "openfaceextractor.h"

#include <dlib/opencv.h>

// TEST!
//#include <opencv2/highgui.hpp>	
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_io.h>


class OpenFaceDescriptorComputer //: DnnFaceDescriptorComputer<OpenFace, DlibFaceExtractor>
{
	// TODO: fix paths
	static inline const std::string faceRecognitionModel{ "./nn4.v2.t7" };	
	static inline const std::string landmarkDetectionModel{ "./shape_predictor_68_face_landmarks.dat" };

public:

	//using Descriptor = OpenFace::OutputLabel;
	//class Descriptor;
	using Descriptor = OpenFace::Descriptor;
	

	//// This is an alternative to std::is_nothrow_constructible (slightly shorter in this case). 
	//// Since we don't care about the destructor, we use "new" rather than T(arg). Also it's a global placement new
	//// (thus we ignore memory allocation exceptions). In many implementations is_nothrow_constructible is 
	//// effectively noexcept(T(arg)) though.
	////OpenFaceDescriptorComputer() noexcept(noexcept(::new (nullptr) DnnFaceDescriptorComputer(faceRecognitionModel, landmarkDetectionModel)))


	/*OpenFaceDescriptorComputer() noexcept(std::is_nothrow_constructible_v<DnnFaceDescriptorComputer<OpenFace, DlibFaceExtractor>
										, decltype(faceRecognitionModel), decltype(landmarkDetectionModel)>)
		: DnnFaceDescriptorComputer(faceRecognitionModel, DlibFaceExtractor{ landmarkDetectionModel, OpenFace::inputImageSize, 0.25 })
	{ }*/

	OpenFaceDescriptorComputer()
		: faceRecognizer(faceRecognitionModel)
		//, faceExtractor(landmarkDetectionModel, 96, 0.2)	// TODO: what's the size?
		, faceExtractor(landmarkDetectionModel)	// TODO: what's the size?
	{

	}

	// TODO: define copy/move semantics

	template <typename Input>
	std::optional<Descriptor> operator ()(Input&& input)
	{
		/*
		// TEST!
		dlib::frontal_face_detector faceDetector = dlib::get_frontal_face_detector();
		dlib::shape_predictor landmarkDetector;
		dlib::deserialize("./shape_predictor_5_face_landmarks.dat") >> landmarkDetector;
		
		dlib::array2d<dlib::rgb_pixel> imDlib;
		std::string imagePath = input;
		dlib::load_image(imDlib, imagePath);

		std::vector<dlib::rectangle> faceRects = faceDetector(imDlib);

		auto r = faceRects[0];
		dlib::full_object_detection landmarks = landmarkDetector(imDlib, r);
		assert(landmarks.num_parts() > 0);

		dlib::matrix<dlib::rgb_pixel> faceChip;
		dlib::extract_image_chip(imDlib, dlib::get_face_chip_details(landmarks, 96, 0.2), faceChip);

		//dlib::image_window testwin(faceChip, "dlibtest");
		//testwin.wait_until_closed();

		const cv::Mat& tmp = dlib::toMat(faceChip);
		cv::Mat aligned;
		tmp.copyTo(aligned);

		return this->faceRecognizer(aligned, false);
		*/

		//// TEST!
		//if (std::optional<DlibFaceExtractor::Output> face = faceExtractor(std::forward<Input>(input)); face)
		//{			
		//	//const cv::Mat& faceMat = dlib::toMat(*face);	// const reference prolongs life of a temporary
		//	const cv::Mat& faceMat = dlib::toMat(*face);	// const reference prolongs life of a temporary
		//	//cv::imshow("test", faceMat);
		//	//cv::waitKey();
		//	cv::Mat tmp;
		//	faceMat.copyTo(tmp);
		//	return this->faceRecognizer(tmp, false);		// caffe models normally use BGR order
		//}
		

		if (std::optional<OpenFaceExtractor::Output> face = faceExtractor(std::forward<Input>(input), 96); face)
		{
			//cv::imshow("test", *face);
			//cv::waitKey(5000);
			return this->faceRecognizer(*face, true);
		}

		return std::nullopt;
		

		// TODO: final code:
		//std::optional<DlibFaceExtractor::Output> face = faceExtractor(std::forward<Input>(input));
		//return face ? this->faceRecognizer(dlib::toMat(*face), true) : std::nullopt;
	}

	//using Network = OpenFace;
	//using DnnFaceDescriptorComputer::FaceExtractor;

private:
	OpenFace faceRecognizer;
	//DlibFaceExtractor faceExtractor;
	OpenFaceExtractor faceExtractor;
};	// OpenFaceDescriptorComputer





#endif	// OPENFACEDESCRIPTORCOMPUTER_H