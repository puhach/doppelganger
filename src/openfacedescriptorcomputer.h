#ifndef OPENFACEDESCRIPTORCOMPUTER_H
#define OPENFACEDESCRIPTORCOMPUTER_H


#include "openface.h"
//#include "dnnfacedescriptorcomputer.h"
//#include "openfacedescriptor.h"
#include "dlibfaceextractor.h"

#include <dlib/opencv.h>

//#include <opencv2/highgui.hpp>	// TEST!


class OpenFaceDescriptorComputer //: DnnFaceDescriptorComputer<OpenFace, DlibFaceExtractor>
{
	// TODO: fix paths
	static inline const std::string faceRecognitionModel{ "./nn4.small2.v1.t7" };	
	static inline const std::string landmarkDetectionModel{ "./shape_predictor_5_face_landmarks.dat" };

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
		, faceExtractor(landmarkDetectionModel, 200, 0.2)	// TODO: what's the size?
	{

	}

	// TODO: define copy/move semantics

	template <typename Input>
	std::optional<Descriptor> operator ()(Input&& input)
	{
		/*
		// TEST!
		if (std::optional<DlibFaceExtractor::Output> face = faceExtractor(std::forward<Input>(input)); face)
		{			
			//const cv::Mat& faceMat = dlib::toMat(*face);	// const reference prolongs life of a temporary
			const cv::Mat& faceMat = dlib::toMat(*face);	// const reference prolongs life of a temporary
			cv::imshow("test", faceMat);
			cv::waitKey();
			return this->faceRecognizer(faceMat, true);		// caffe models normally use BGR order
		}
		
		return std::nullopt;
		*/

		// TODO: final code:
		std::optional<DlibFaceExtractor::Output> face = faceExtractor(std::forward<Input>(input));
		return face ? this->faceRecognizer(dlib::toMat(*face), true) : std::nullopt;
	}

	//using Network = OpenFace;
	//using DnnFaceDescriptorComputer::FaceExtractor;

private:
	OpenFace faceRecognizer;
	DlibFaceExtractor faceExtractor;
};	// OpenFaceDescriptorComputer





#endif	// OPENFACEDESCRIPTORCOMPUTER_H