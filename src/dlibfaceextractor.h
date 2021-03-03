#ifndef DLIBFACEEXTRACTOR_H
#define DLIBFACEEXTRACTOR_H

#include "faceextractorhelper.h"

#include <optional>
#include <string>
#include <execution>
#include <atomic>

#include <dlib/matrix.h>
//#include <dlib/pixel.h>


// TODO: perhaps, rename it to StockDlibFaceExtractor?
template <typename PixelType>
//class DlibFaceExtractor : FaceExtractorHelper<dlib::matrix<dlib::rgb_pixel>>
class DlibFaceExtractor : FaceExtractorHelper<dlib::matrix<PixelType>>
{
public:
	
	using typename FaceExtractorHelper::Output;
	//using FaceExtractorHelper<dlib::matrix<PixelType>>::Output;
	//using Output = typename FaceExtractorHelper<dlib::matrix<PixelType>>::Output;

	// DlibFaceExtractor works with both 5 and 68 landmark detection models
	DlibFaceExtractor(const std::string& landmarkDetectionModel, unsigned long size, double padding = 0.2)
		: FaceExtractorHelper(landmarkDetectionModel, [this](const std::string& filePath) { return extractFace(filePath); })
		, size(size > 0 ? size : throw std::invalid_argument("Image size cannot be zero."))
		, padding(padding >= 0 ? padding : throw std::invalid_argument("Padding cannot be negative."))	{	}

	unsigned long getSize() const noexcept { return size; }
	void setSize(unsigned long size) noexcept { this->size = size; }

	double getPadding() const noexcept { return this->padding; }
	void setPadding() noexcept { this->padding = padding; }


	// Private inheritance prevents slicing and accidental deletion via base class pointer

	DlibFaceExtractor(const DlibFaceExtractor& other) = default;
	DlibFaceExtractor(DlibFaceExtractor&& other) = default;

	DlibFaceExtractor& operator = (const DlibFaceExtractor& other) = default;
	DlibFaceExtractor& operator = (DlibFaceExtractor&& other) = default;

	using FaceExtractorHelper::operator();

private:

	std::optional<Output> extractFace(const std::string& filePath);
	//std::optional<typename Output> extractFace(const std::string& filePath);
	//std::optional<typename DlibFaceExtractor<PixelType>::Output> extractFace(const std::string& filePath);

	unsigned long size;
	double padding;
};	// DlibFaceExtractor

template <typename PixelType>
std::optional<typename DlibFaceExtractor<PixelType>::Output> DlibFaceExtractor<PixelType>::extractFace(const std::string& filePath)
{
	// Load the image
	dlib::array2d<PixelType> im;	// TODO: array or matrix
	dlib::load_image(im, filePath);

	auto landmarks = FaceExtractorHelper::getLandmarks(im);		// call the inherited helper function
	if (landmarks.num_parts() < 1)
		return std::nullopt;

	// Align the face
	dlib::matrix<PixelType> face;		// TODO: matrix vs array2d
	//dlib::extract_image_chip(im, dlib::get_face_chip_details(landmarks, 256, 0.25), face);	// TODO: add class parameters
	//dlib::extract_image_chip(im, dlib::get_face_chip_details(landmarks, inputImageSize<ResNet>, 0.25), face);
	dlib::extract_image_chip(im, dlib::get_face_chip_details(landmarks, this->size, this->padding), face);

	return std::move(face);		// prefer move-constructor for std::optional
}	// extractFace



#endif	// DLIBFACEEXTRACTOR_H
