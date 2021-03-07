#ifndef RESNETFACEDESCRIPTORCOMPUTER_H
#define RESNETFACEDESCRIPTORCOMPUTER_H

#include "resnet.h"
#include "facedescriptorcomputer.h"
#include "dlibfaceextractor.h"

#include <tuple>


/*
* ResNetFaceDescriptorComputer is a callable object that takes in an one or more input image files, crops the face by means of DlibFaceExtractor,
* and computes face descriptor(s) using ResNet neural network. 
* 
* This class cannot be copied, moved, or destroyed using a pointer or reference to the base class.
*/

class ResNetFaceDescriptorComputer : public FaceDescriptorComputer<DlibFaceExtractor<ResNet::Input>, ResNet>
{
public:
	ResNetFaceDescriptorComputer(const std::string& landmarkDetectionModel, const std::string& faceRecognitionModel, double padding = 0.25)
		: FaceDescriptorComputer(std::forward_as_tuple(landmarkDetectionModel, ResNet::inputSize, padding)
								, std::forward_as_tuple(faceRecognitionModel)) { }

	ResNetFaceDescriptorComputer(const ResNetFaceDescriptorComputer& other) = default;
	ResNetFaceDescriptorComputer(ResNetFaceDescriptorComputer&& other) = default;

	ResNetFaceDescriptorComputer& operator = (const ResNetFaceDescriptorComputer& other) = default;
	ResNetFaceDescriptorComputer& operator = (ResNetFaceDescriptorComputer&& other) = default;
};	// ResNetFaceDescriptorComputer


// namespace face_descriptor_traits
// {
//     template <> const std::string inline descriptorComputerTypeId<ResNetFaceDescriptorComputer> = "ResNetFaceDescriptorComputer";    
// };

template <>
struct DescriptorComputerType<ResNetFaceDescriptorComputer>
{
    static const inline std::string id = "ResNetFaceDescriptorComputer";
};

#endif	// RESNETFACEDESCRIPTORCOMPUTER_H
