#ifndef DLIBFACEEXTRACTOR_H
#define DLIBFACEEXTRACTOR_H

#include "faceextractorhelper.h"

#include <optional>
#include <string>
#include <execution>	
#include <atomic>

//#include <dlib/matrix.h>
//#include <dlib/pixel.h>
#include <dlib/image_io.h>
#include <dlib/image_transforms.h>


//template <typename PixelType>
//class DlibFaceExtractor : FaceExtractorHelper<dlib::matrix<PixelType>>
template <class Image>
class DlibFaceExtractor : FaceExtractorHelper<Image>
{
	//static_assert(std::is_invocable_v<decltype(dlib::load_image<Image>), Image, std::string>, "The specified image type is not supported by Dlib.");
	using typename DlibFaceExtractor::FaceExtractorHelper::ExtractFaceCallback;
public:

	using typename DlibFaceExtractor::FaceExtractorHelper::Output;
	//using typename FaceExtractorHelper::Output;
	//using FaceExtractorHelper<dlib::matrix<PixelType>>::Output;
	//using Output = typename FaceExtractorHelper<dlib::matrix<PixelType>>::Output;
	//typedef std::optional<Output>(FaceExtractorHelper<Image>::*ExtractorCallback)(const std::string& filePath);

	// DlibFaceExtractor works with both 5 and 68 landmark detection models
	DlibFaceExtractor(const std::string& landmarkDetectionModel, unsigned long size, double padding = 0.2)
		: DlibFaceExtractor::FaceExtractorHelper(landmarkDetectionModel, static_cast<ExtractFaceCallback>(&DlibFaceExtractor::extractFace))
		, size(size > 0 ? size : throw std::invalid_argument("Image size cannot be zero."))
		, padding(padding >= 0 ? padding : throw std::invalid_argument("Padding cannot be negative.")) {	}

	unsigned long getSize() const noexcept { return size; }
	void setSize(unsigned long size) noexcept { this->size = size; }

	double getPadding() const noexcept { return this->padding; }
	void setPadding() noexcept { this->padding = padding; }


	// Private inheritance prevents slicing and accidental deletion via base class pointer

	DlibFaceExtractor(const DlibFaceExtractor& other) = default;
	DlibFaceExtractor(DlibFaceExtractor&& other) = default;

	DlibFaceExtractor& operator = (const DlibFaceExtractor& other) = default;
	DlibFaceExtractor& operator = (DlibFaceExtractor&& other) = default;

	using DlibFaceExtractor::FaceExtractorHelper::operator();

private:

	std::optional<Output> extractFace(const std::string& filePath);
	//std::optional<typename Output> extractFace(const std::string& filePath);
	//std::optional<typename DlibFaceExtractor<PixelType>::Output> extractFace(const std::string& filePath);

	unsigned long size;
	double padding;
};	// DlibFaceExtractor


template <class Image>
std::optional<typename DlibFaceExtractor<Image>::Output> DlibFaceExtractor<Image>::extractFace(const std::string& filePath)
{
	// Load the image
	Image im;
	dlib::load_image(im, filePath);

	// Obtain the coordinates of facial landmarks
	auto landmarks = DlibFaceExtractor::FaceExtractorHelper::getLandmarks(im);		// call the inherited helper function
	if (landmarks.num_parts() < 1)
		return std::nullopt;

	// Align the face
	Image face;
	dlib::extract_image_chip(im, dlib::get_face_chip_details(landmarks, this->size, this->padding), face);

	return std::move(face);		// prefer move-constructor for std::optional
}	// extractFace



#endif	// DLIBFACEEXTRACTOR_H
