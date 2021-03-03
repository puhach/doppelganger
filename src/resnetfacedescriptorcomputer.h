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

// TODO: try 5-landmark detector
//#define SPECIALIZE_RESNETDESCRIPTORCOMPUTER_CTOR(pixel_type)	\
//	template <>	\
//	template <>	\
//	DnnFaceDescriptorComputer<DlibFaceExtractor<pixel_type>, ResNet>::DnnFaceDescriptorComputer()	\
//		: faceExtractor("./shape_predictor_68_face_landmarks.dat", ResNet::inputImageSize, 0.25)	\
//		, faceRecognizer("./dlib_face_recognition_resnet_model_v1.dat") {}	
//
////SPECIALIZE_RESNETDESCRIPTORCOMPUTER_CTOR(dlib::rgb_pixel)
//SPECIALIZE_RESNETDESCRIPTORCOMPUTER_CTOR(dlib::image_traits<ResNet::Input>::pixel_type)

template <>
template <>
DnnFaceDescriptorComputer<DlibFaceExtractor<ResNet::PixelType>, ResNet>::DnnFaceDescriptorComputer()
	// TODO: fix paths
	: faceExtractor("./shape_predictor_5_face_landmarks.dat", ResNet::inputImageSize, 0.25)
	, faceRecognizer("./dlib_face_recognition_resnet_model_v1.dat") {}


using ResNetFaceDescriptorComputer = DnnFaceDescriptorComputer<DlibFaceExtractor<ResNet::PixelType>, ResNet>;

//using ResNetFaceDescriptorComputer = DnnFaceDescriptorComputer<DlibFaceExtractor<dlib::rgb_pixel>, ResNet>;
//using ResNetFaceDescriptorComputer = DnnFaceDescriptorComputer<DlibFaceExtractor<dlib::bgr_pixel>, ResNet>;

/*
template <>
template <>
DnnFaceDescriptorComputer<DlibFaceExtractor, ResNet>::DnnFaceDescriptorComputer()
	// TODO: try 5-landmark detector
	: faceExtractor("./shape_predictor_68_face_landmarks.dat", ResNet::inputImageSize, 0.25)
	, faceRecognizer("./dlib_face_recognition_resnet_model_v1.dat") {}


using ResNetFaceDescriptorComputer = DnnFaceDescriptorComputer<DlibFaceExtractor, ResNet>;
*/

#endif	// RESNETFACEDESCRIPTORCOMPUTER_H
