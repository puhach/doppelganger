#ifndef DNNDESCRIPTORCOMPUTER_H
#define DNNDESCRIPTORCOMPUTER_H

#include <optional>
#include <string>

#include <dlib/image_processing.h>


template <class Network, class DescriptorT = Network::output_label_type>
class DnnFaceDescriptorComputer
{
public:

	using Descriptor = DescriptorT;

	template <typename ... Args>
	DnnFaceDescriptorComputer(const std::string& landmarkDetectionModel, const std::string& faceRecognitionModel, Args ... args)
		: net(std::forward(args)...)
	{
		dlib::deserialize(landmarkDetectionModel) >> this->landmarkDetector;
		dlib::deserialize(faceRecognitionModel) >> this->net;
	}

	// TODO: define copy/move semantics

	std::optional<Descriptor> computeDescriptor(const std::string& filePath);

private:
	dlib::frontal_face_detector faceDetector = dlib::get_frontal_face_detector();
	dlib::shape_predictor landmarkDetector;
	Network net;
	static const inline dlib::pyramid_down<2> pyrDown;		// TODO: common
};	// DnnFaceDescriptorComputer

#include "dnnfacedescriptorcomputer_impl.h"



#endif	//DNNDESCRIPTORCOMPUTER_H