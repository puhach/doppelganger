#include "dlibfaceextractor.h"

#include <dlib/image_io.h>
#include <dlib/image_processing.h>


std::optional<DlibFaceExtractor::Output> DlibFaceExtractor::operator() (const std::string& filePath)
{
	// Load the image
	dlib::array2d<dlib::rgb_pixel> im;
	dlib::load_image(im, filePath);

	// Detect the face
	// TODO: downsample the image
	static const dlib::pyramid_down<2> pyrDown;		// TODO: common
	dlib::array2d<dlib::rgb_pixel> imDown;	// TODO: common?
	//if (double scale = std::max(im.nr(), im.nc()) / 300.0; scale > 1)
	bool downsampled = false;
	if (std::max(im.nr(), im.nc()) > 300)
	{
		pyrDown(im, imDown);
		im.swap(imDown);
		downsampled = true;
	}

	//thread_local dlib::frontal_face_detector faceDetector = faceDetectorOrigin;		// copying is faster calling get_frontal_face_detector() every time
	auto faces = this->faceDetector(im);
	if (faces.empty())		// no faces detected in this image
		return std::nullopt;

	dlib::rectangle faceRect = faces[0];
	//if (imDown.begin() != imDown.end())		// scale back
	if (downsampled)
	{
		im.swap(imDown);
		faceRect = pyrDown.rect_up(faceRect);
	}

	// From Dlib docs: No synchronization is required when using this object.  In particular, a
	// single instance of this object can be used from multiple threads at the same time.
	//auto landmarks = landmarkDetector(im, faces[0]);	// TODO: remember to scale back the face rectangle
	auto landmarks = this->landmarkDetector(im, faceRect);
	if (landmarks.num_parts() < 1)
		return std::nullopt;		// TODO: perhaps, define a specific exception for detection failure?

	// Align the face
	dlib::matrix<dlib::rgb_pixel> face;		// TODO: matrix vs array2d
	//dlib::extract_image_chip(im, dlib::get_face_chip_details(landmarks, 256, 0.25), face);	// TODO: add class parameters
	//dlib::extract_image_chip(im, dlib::get_face_chip_details(landmarks, inputImageSize<ResNet>, 0.25), face);
	dlib::extract_image_chip(im, dlib::get_face_chip_details(landmarks, this->size, this->padding), face);

	return face;
}