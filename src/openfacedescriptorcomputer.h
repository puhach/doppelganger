#ifndef OPENFACEDESCRIPTORCOMPUTER_H
#define OPENFACEDESCRIPTORCOMPUTER_H

#include "facedescriptorcomputer.h"
#include "openface.h"
//#include "openfacedescriptor.h"
//#include "dlibfaceextractor.h"
#include "openfaceextractor.h"

#include <tuple>

//// TODO: fix paths	
//#define SPECIALIZE_OPENFACEDESCRIPTORCOMPUTER_CTOR(alignment) \
//	template <>	\
//	template <>	\
//	FaceDescriptorComputer<OpenFaceExtractor<alignment>, OpenFace>::FaceDescriptorComputer(	\
//		const std::string &landmarkDetectionModel, const std::string& faceRecognitionModel)	\
//		: faceExtractor(landmarkDetectionModel, OpenFace::inputSize)	\
//		, faceRecognizer(faceRecognitionModel, false) {}	
//
//SPECIALIZE_OPENFACEDESCRIPTORCOMPUTER_CTOR(OpenFaceAlignment::InnerEyesAndBottomLip)
//SPECIALIZE_OPENFACEDESCRIPTORCOMPUTER_CTOR(OpenFaceAlignment::OuterEyesAndNose)
//
//// TODO: 'using' can also be templated
//// OpenFace suggests using outerEyesAndNose alignment:
//// https://cmusatyalab.github.io/openface/visualizations/#2-preprocess-the-raw-images
//using OpenFaceDescriptorComputer = FaceDescriptorComputer<OpenFaceExtractor<OpenFaceAlignment::OuterEyesAndNose>, OpenFace>;

template <OpenFaceAlignment alignment>
class OpenFaceDescriptorComputer : public FaceDescriptorComputer<OpenFaceExtractor<alignment>, OpenFace>
{
public:
	// OpenFaceExtractor requires a 68-landmark detection model
	OpenFaceDescriptorComputer(const std::string& landmarkDetectionModel, const std::string& faceRecognitionModel)
		: FaceDescriptorComputer(std::forward_as_tuple(landmarkDetectionModel, OpenFace::inputSize)
								, std::forward_as_tuple(faceRecognitionModel, false)) {}
};	// OpenFaceDescriptorComputer


#endif	// OPENFACEDESCRIPTORCOMPUTER_H