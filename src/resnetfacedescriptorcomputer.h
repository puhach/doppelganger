#ifndef RESNETFACEDESCRIPTORCOMPUTER_H
#define RESNETFACEDESCRIPTORCOMPUTER_H

#include "resnet.h"
#include "dnnfacedescriptorcomputer.h"
#include "dlibfaceextractor.h"

//using ResNetFaceDescriptorComputer = DnnFaceDescriptorComputer<ResNet, DlibFaceExtractor>;

class ResNetFaceDescriptorComputer : DnnFaceDescriptorComputer<ResNet, DlibFaceExtractor>	// private inheritance prevents slicing
{
public:
	// This is an alternative to std::is_nothrow_constructible (slightly shorter in this case). 
	// Since we don't care about the destructor, we use "new" rather than T(arg). Also it's a global placement new
	// (thus we ignore memory allocation exceptions). In many implementations is_nothrow_constructible is 
	// effectively noexcept(T(arg)) though.
	ResNetFaceDescriptorComputer() noexcept(noexcept(::new (nullptr) DnnFaceDescriptorComputer(faceRecognitionModel, landmarkDetectionModel)))
		: DnnFaceDescriptorComputer(faceRecognitionModel, DlibFaceExtractor{ landmarkDetectionModel, ResNet::inputImageSize, 0.25 })
	{ }
	/*template <class ResNetT, class FaceExtractorT>
	ResNetFaceDescriptorComputer(ResNetT&& resnet, FaceExtractorT&& faceExtractor)
		: DnnFaceDescriptor(std::forward<ResNetT>(resnet), std::forward<FaceExtractorT>(faceExtractor))
	{

	}*/
	
	// TODO: define copy/move semantics

	using DnnFaceDescriptorComputer::operator ();

	using DnnFaceDescriptorComputer::Network;
	using DnnFaceDescriptorComputer::FaceExtractor;
	using DnnFaceDescriptorComputer::Descriptor;

private:
	//ResNet resnet{"./dlib_face_recognition_resnet_model_v1.dat"};
	//DlibFaceExtractor faceExtractor{"./shape_predictor_68_face_landmarks.dat"};
	static inline const std::string faceRecognitionModel{ "./dlib_face_recognition_resnet_model_v1.dat" };
	static inline const std::string landmarkDetectionModel{ "./shape_predictor_68_face_landmarks.dat" };
};	// ResNetFaceDescriptorComputer

#endif	// RESNETFACEDESCRIPTORCOMPUTER_H
