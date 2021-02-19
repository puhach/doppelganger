#ifndef DLIBFACEEXTRACTOR_H
#define DLIBFACEEXTRACTOR_H

#include <optional>
#include <string>

#include <dlib/array2d.h>
#include <dlib/pixel.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/shape_predictor.h>


class DlibFaceExtractor
{
public:
	//using Output = dlib::array2d<dlib::rgb_pixel>;
	using Output = dlib::matrix<dlib::rgb_pixel>;

	DlibFaceExtractor(const std::string& landmarkDetectionModel, unsigned long size, double padding = 0.2)
		: size(size > 0 ? size : throw std::invalid_argument("Image size cannot be zero."))
		, padding(padding >= 0 ? padding : throw std::invalid_argument("Padding cannot be negative."))
	{
		dlib::deserialize(landmarkDetectionModel) >> this->landmarkDetector;
	}

	unsigned long getSize() const noexcept { return size; }
	void setSize(unsigned long size) noexcept { this->size = size; }

	double getPadding() const noexcept { return this->padding; }
	void setPadding() noexcept { this->padding = padding; }

	// TODO: define copy/move semantics

	std::optional<Output> operator() (const std::string& filePath);	
	//dlib::matrix<dlib::rgb_pixel>& operator() (const std::string& filePath);	// TEST!

	// TODO: perhaps, add an overload for matrix/array2d? 

private:
	dlib::frontal_face_detector faceDetector = dlib::get_frontal_face_detector();
	dlib::shape_predictor landmarkDetector;
	unsigned long size;
	double padding;
};	// DlibFaceExtractor

#endif	// DLIBFACEEXTRACTOR_H
