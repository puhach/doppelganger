/*
#include "dlibfaceextractor.h"

#include <dlib/image_io.h>
#include <dlib/image_processing.h>


std::optional<DlibFaceExtractor::Output> DlibFaceExtractor::extractFace(const std::string& filePath)
{
	// Load the image
	dlib::array2d<dlib::rgb_pixel> im;	// TODO: array or matrix
	dlib::load_image(im, filePath);

	auto landmarks = FaceExtractorHelper::getLandmarks(im);
	if (landmarks.num_parts() < 1)
		return std::nullopt;

	// Align the face
	dlib::matrix<dlib::rgb_pixel> face;		// TODO: matrix vs array2d
	//dlib::extract_image_chip(im, dlib::get_face_chip_details(landmarks, 256, 0.25), face);	// TODO: add class parameters
	//dlib::extract_image_chip(im, dlib::get_face_chip_details(landmarks, inputImageSize<ResNet>, 0.25), face);
	dlib::extract_image_chip(im, dlib::get_face_chip_details(landmarks, this->size, this->padding), face);

	return std::move(face);		// prefer move-constructor for std::optional
}	// extractFace

*/
