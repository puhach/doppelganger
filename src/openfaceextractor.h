#ifndef OPENFACEEXTRACTOR_H
#define OPENFACEEXTRACTOR_H

#include <string>
#include <optional>

#include <opencv2/core.hpp>
//#include <dlib/array2d.h>
//#include <dlib/pixel.h>
#include <dlib/matrix.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/shape_predictor.h>


class OpenFaceExtractor
{
public:
	
	using Output = cv::Mat;

	OpenFaceExtractor(const std::string& landmarkDetectionModel);

	std::optional<Output> operator()(const std::string& filePath, unsigned long size);

private:
	dlib::frontal_face_detector faceDetector = dlib::get_frontal_face_detector();
	dlib::shape_predictor landmarkDetector;
};	// OpenFaceExtractor

#endif	// OPENFACEEXTRACTOR_H
