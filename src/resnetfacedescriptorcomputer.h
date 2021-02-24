#ifndef RESNETFACEDESCRIPTORCOMPUTER_H
#define RESNETFACEDESCRIPTORCOMPUTER_H

#include "resnet.h"
#include "dnnfacedescriptorcomputer.h"
#include "dlibfaceextractor.h"

//using ResNetFaceDescriptorComputer = DnnFaceDescriptorComputer<ResNet, DlibFaceExtractor>;

/*
* ResNetFaceDescriptorComputer is a default-constructible class for computing face descriptors using ResNet neural network.
* The face is cropped from an input image by means of DlibFaceExtractor. 
* 
* Private inheritance prevents slicing and accidental deletion via base class pointer. On the other hand, when subclassed
* privately, ResNetFaceDescriptorComputer is not treated as a type of DnnFaceDescriptorComputer<ResNet, DlibFaceExtractor>, 
* so we won't be able to pass it to a function taking a pointer/reference to DnnFaceDescriptorComputer<ResNet, DlibFaceExtractor>. 
* Alternatively, we could define a virtual copy interface and make DnnFaceDescriptorComputer destructor virtual, but that
* would lead to performance overhead.
*/

class ResNetFaceDescriptorComputer : DnnFaceDescriptorComputer<ResNet, DlibFaceExtractor>	// TODO: make it final?
{
    // TODO: try 5-landmark detector
	static inline const std::string faceRecognitionModel{ "./dlib_face_recognition_resnet_model_v1.dat" };
	static inline const std::string landmarkDetectionModel{ "./shape_predictor_68_face_landmarks.dat" };
public:
	// This is an alternative to std::is_nothrow_constructible (slightly shorter in this case). 
	// Since we don't care about the destructor, we use "new" rather than T(arg). Also it's a global placement new
	// (thus we ignore memory allocation exceptions). In many implementations is_nothrow_constructible is 
	// effectively noexcept(T(arg)) though.
	ResNetFaceDescriptorComputer() noexcept(noexcept(::new (nullptr) DnnFaceDescriptorComputer(faceRecognitionModel, landmarkDetectionModel)))
		: DnnFaceDescriptorComputer(faceRecognitionModel, DlibFaceExtractor{ landmarkDetectionModel, ResNet::inputImageSize, 0.25 })
	{ }

	
	// TODO: define copy/move semantics

	using DnnFaceDescriptorComputer::operator ();

	using DnnFaceDescriptorComputer::Network;
	using DnnFaceDescriptorComputer::FaceExtractor;
	using DnnFaceDescriptorComputer::Descriptor;

private:
	
};	// ResNetFaceDescriptorComputer

#endif	// RESNETFACEDESCRIPTORCOMPUTER_H
