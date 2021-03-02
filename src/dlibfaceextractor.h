#ifndef DLIBFACEEXTRACTOR_H
#define DLIBFACEEXTRACTOR_H

#include "faceextractorhelper.h"

#include <optional>
#include <string>
#include <execution>
#include <atomic>

#include <dlib/matrix.h>
#include <dlib/pixel.h>


// TODO: perhaps, rename it to StockDlibFaceExtractor?
class DlibFaceExtractor : FaceExtractorHelper<dlib::matrix<dlib::rgb_pixel>>
{
public:
	
	using FaceExtractorHelper::Output;

	// DlibFaceExtractor works with both 5 and 68 landmark detection models
	DlibFaceExtractor(const std::string& landmarkDetectionModel, unsigned long size, double padding = 0.2)
		: FaceExtractorHelper(landmarkDetectionModel, [this](const std::string& filePath) { return extractFace(filePath); })
		, size(size > 0 ? size : throw std::invalid_argument("Image size cannot be zero."))
		, padding(padding >= 0 ? padding : throw std::invalid_argument("Padding cannot be negative."))
	{		
	}

	unsigned long getSize() const noexcept { return size; }
	void setSize(unsigned long size) noexcept { this->size = size; }

	double getPadding() const noexcept { return this->padding; }
	void setPadding() noexcept { this->padding = padding; }

	// TODO: define copy/move semantics

	using FaceExtractorHelper::operator();

private:

	std::optional<Output> extractFace(const std::string& filePath);

	unsigned long size;
	double padding;
};	// DlibFaceExtractor




/*
class DlibFaceExtractor
{
public:
	//using Output = dlib::array2d<dlib::rgb_pixel>;
	using Output = dlib::matrix<dlib::rgb_pixel>;

	// TODO: size and padding should, probably, be specified in the call operator?
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

	template <class InputIterator, class OutputIterator>
	OutputIterator operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead);

private:

	static const dlib::frontal_face_detector& getFaceDetector()
	{
		static const dlib::frontal_face_detector faceDetector = dlib::get_frontal_face_detector();
		return faceDetector;
	}

	//dlib::frontal_face_detector faceDetector = dlib::get_frontal_face_detector();
	dlib::shape_predictor landmarkDetector;		// Dlib's landmark detector is said to be thread-safe
	unsigned long size;
	double padding;
};	// DlibFaceExtractor


template <class InputIterator, class OutputIterator>
OutputIterator DlibFaceExtractor::operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead)
{
	assert(inHead <= inTail);

	//std::vector<dlib::matrix<dlib::rgb_pixel>> images(inTail-inHead);
	//std::transform(inHead, inTail, images.begin(), [](const auto& filePath) 
	//	{
	//		// TODO: exceptions
	//		dlib::matrix<dlib::rgb_pixel> im;
	//		dlib::load_image(im, filePath);
	//		return im;
	//	});

	//auto faceDetector = getFaceDetector();
	////std::vector<std::optional<dlib::rectangle>> faceRects(images.size());
	//std::vector<std::vector<dlib::rectangle>> faceRects(images.size());
	//faceDetector(images.begin(), images.end(), faceRects.begin());

	std::atomic_flag eflag{ false };
	std::exception_ptr eptr;
	outHead = std::transform(std::execution::par, inHead, inTail, outHead, 
		[this, &eflag, &eptr](const auto& filePath) -> std::optional<Output>
		{
			try
			{
				//thread_local auto localDetector = getFaceDetector();
				return (*this)(filePath);
			}
			catch (...)
			{
				if (!eflag.test_and_set(std::memory_order_acq_rel))
					eptr = std::current_exception();
			}

			return std::nullopt;
		});	// transform

	if (eptr)
		std::rethrow_exception(eptr);
		
	return outHead;
}
*/

#endif	// DLIBFACEEXTRACTOR_H
