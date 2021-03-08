#ifndef OPENFACEDESCRIPTORCOMPUTER_H
#define OPENFACEDESCRIPTORCOMPUTER_H

#include "facedescriptorcomputer.h"
#include "openface.h"
#include "openfaceextractor.h"

#include <tuple>


/*
* OpenFaceDescriptorComputer is a callable object that takes in an one or more input image files, crops the face by means of the dedicated
* OpenFaceExtractor, and computes face descriptor(s) using OpenFace model.
*
* This class cannot be copied, moved, or destroyed using a pointer or reference to the base class.
*/

template <OpenFaceAlignment alignment>
class OpenFaceDescriptorComputer : public FaceDescriptorComputer<OpenFaceExtractor<alignment>, OpenFace>
{
public:
	// OpenFaceExtractor requires a 68-landmark detection model
	OpenFaceDescriptorComputer(const std::string& landmarkDetectionModel, const std::string& faceRecognitionModel)
		: OpenFaceDescriptorComputer::FaceDescriptorComputer(std::forward_as_tuple(landmarkDetectionModel, OpenFace::inputSize)
															, std::forward_as_tuple(faceRecognitionModel, false)) {}

	OpenFaceDescriptorComputer(const OpenFaceDescriptorComputer& other) = default;
	OpenFaceDescriptorComputer(OpenFaceDescriptorComputer&& other) = default;

	OpenFaceDescriptorComputer& operator = (const OpenFaceDescriptorComputer& other) = default;
	OpenFaceDescriptorComputer& operator = (OpenFaceDescriptorComputer&& other) = default;
    
    //static const inline std::string typeName = "OpenFaceDescriptorComputer_" + std::to_string(static_cast<int>(alignment));
};	// OpenFaceDescriptorComputer

// namespace face_descriptor_traits
// {
//     template <> const std::string inline descriptorComputerTypeId<OpenFaceDescriptorComputer<OpenFaceAlignment::InnerEyesAndBottomLip>> = "OpenFaceDescriptorComputer_InnerEyesAndBottomLip";
//     template <> const std::string inline descriptorComputerTypeId<OpenFaceDescriptorComputer<OpenFaceAlignment::OuterEyesAndNose>> = "OpenFaceDescriptorComputer_OuterEyesAndNose";
// };

template <>
struct DescriptorComputerType<OpenFaceDescriptorComputer<OpenFaceAlignment::InnerEyesAndBottomLip>>
{
    static const inline std::string id = "OpenFaceDescriptorComputer_InnerEyesAndBottomLip";
};

template <>
struct DescriptorComputerType<OpenFaceDescriptorComputer<OpenFaceAlignment::OuterEyesAndNose>>
{
    static const inline std::string id = "OpenFaceDescriptorComputer_OuterEyesAndNose";
};

#endif	// OPENFACEDESCRIPTORCOMPUTER_H
