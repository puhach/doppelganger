#ifndef FACEDB_H
#error Do not include dnnfacedescriptorcomputer_impl.h directly
#endif

#include <dlib/image_processing.h>
#include <dlib/dnn.h>


template <class Network, class DescriptorT>
//typename DnnFaceDescriptorComputer<Network, DescriptorT>::Descriptor DnnFaceDescriptorComputer<Network, DescriptorT>::computeDescriptor(const std::string& filePath)
std::optional<DescriptorT> DnnFaceDescriptorComputer<Network, DescriptorT>::computeDescriptor(const std::string& filePath)
{

	//dlib::shape_predictor landmarkDetector;
	//dlib::deserialize("./shape_predictor_68_face_landmarks.dat") >> landmarkDetector;

	// Load the image
	dlib::array2d<dlib::rgb_pixel> im;
	dlib::load_image(im, filePath);

	// Detect the face
	// TODO: downsample the image
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
	dlib::extract_image_chip(im, dlib::get_face_chip_details(landmarks, inputImageSize<ResNet>, 0.25), face);

	//thread_local ResNet net = netOrigin;
	//dlib::matrix<float,0,1> desc = net(face);
	//ResNet::output_label_type desc = net(face);
	auto descriptor = this->net(face);
	return descriptor;

	//dlib::image_window win(face, "test");
	//win.wait_until_closed();
}
