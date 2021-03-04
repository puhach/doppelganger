#ifndef RESNETFACEDESCRIPTORCOMPUTER_H
#define RESNETFACEDESCRIPTORCOMPUTER_H

#include "resnet.h"
#include "facedescriptorcomputer.h"
#include "dlibfaceextractor.h"




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


class ResNetFaceDescriptorComputer : public FaceDescriptorComputer<DlibFaceExtractor<ResNet::Input>, ResNet>
{
public:
	ResNetFaceDescriptorComputer(const std::string& landmarkDetectionModel, const std::string& faceRecognitionModel, double padding = 0.25)
		: FaceDescriptorComputer(std::forward_as_tuple(landmarkDetectionModel, ResNet::inputSize, padding), 
									std::forward_as_tuple(faceRecognitionModel)) { }

	// TODO: define copy/move semantics
};	// ResNetFaceDescriptorComputer


/*
template <>
template <>
FaceDescriptorComputer<DlibFaceExtractor<ResNet::Input>, ResNet>::FaceDescriptorComputer(
	const std::string& landmarkDetectionModel, const std::string& faceRecognitionModel)
// TODO: fix paths
	: faceExtractor(landmarkDetectionModel, ResNet::inputSize, 0.25)
	, faceRecognizer(faceRecognitionModel) {}

//template <>
//template <>
//FaceDescriptorComputer<DlibFaceExtractor<ResNet::Input>, ResNet>::FaceDescriptorComputer(
//	const char* landmarkDetectionModel, const char* faceRecognitionModel)
//	// TODO: fix paths
//	: faceExtractor(landmarkDetectionModel, ResNet::inputSize, 0.25)
//	, faceRecognizer(faceRecognitionModel) {}


using ResNetFaceDescriptorComputer = FaceDescriptorComputer<DlibFaceExtractor<ResNet::Input>, ResNet>;

*/




#endif	// RESNETFACEDESCRIPTORCOMPUTER_H
