#ifndef OPENFACEDESCRIPTORCOMPUTER_H
#define OPENFACEDESCRIPTORCOMPUTER_H

#include "facedescriptorcomputer.h"
#include "openface.h"
//#include "openfacedescriptor.h"
//#include "dlibfaceextractor.h"
#include "openfaceextractor.h"

//#include <vector>
//#include <optional>
//#include <execution>
//#include <atomic>
//#include <exception>
//
//#include <dlib/opencv.h>

//// TEST!
////#include <opencv2/highgui.hpp>	
//#include <dlib/image_processing.h>
//#include <dlib/image_processing/frontal_face_detector.h>
//#include <dlib/image_io.h>

// TODO: fix paths	
#define SPECIALIZE_OPENFACEDESCRIPTORCOMPUTER_CTOR(alignment) \
	template <>	\
	template <>	\
	FaceDescriptorComputer<OpenFaceExtractor<alignment>, OpenFace>::FaceDescriptorComputer()	\
		: faceExtractor("./shape_predictor_68_face_landmarks.dat", OpenFace::inputImageSize)	\
		, faceRecognizer("./nn4.v2.t7", false) {}	\

SPECIALIZE_OPENFACEDESCRIPTORCOMPUTER_CTOR(OpenFaceAlignment::InnerEyesAndBottomLip)
SPECIALIZE_OPENFACEDESCRIPTORCOMPUTER_CTOR(OpenFaceAlignment::OuterEyesAndNose)


// OpenFace suggests using outerEyesAndNose alignment:
// https://cmusatyalab.github.io/openface/visualizations/#2-preprocess-the-raw-images
using OpenFaceDescriptorComputer = FaceDescriptorComputer<OpenFaceExtractor<OpenFaceAlignment::OuterEyesAndNose>, OpenFace>;



#endif	// OPENFACEDESCRIPTORCOMPUTER_H