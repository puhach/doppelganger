#ifndef DNNDESCRIPTORCOMPUTER_H
#define DNNDESCRIPTORCOMPUTER_H

#include <optional>
#include <string>

//#include <dlib/image_processing.h>	// TEST!


template <class NetworkT, class FaceExtractorT>
class DnnFaceDescriptorComputer
{
	static_assert(std::is_convertible_v<typename FaceExtractorT::Output, typename NetworkT::Input>, "FaceExtractor output type must be compatible with Network input type.");
public:
	using Network = NetworkT;
	using FaceExtractor = FaceExtractorT;
	using Descriptor = typename Network::OutputLabel;

	template <class NetworkT, class FaceExtractorT>
	DnnFaceDescriptorComputer(NetworkT&& network, FaceExtractorT&& faceExtractor) noexcept(
		std::is_nothrow_constructible_v<Network, NetworkT>&& std::is_nothrow_constructible_v<FaceExtractor, FaceExtractorT>)
		: network(std::forward<NetworkT>(network))
		, faceExtractor(std::forward<FaceExtractorT>(faceExtractor))
	{

	}

	// TODO: define copy/move semantics

	template <typename Input>
	//std::optional<typename Network::Descriptor> compute(Input&& input)
	std::optional<Descriptor> operator ()(Input&& input)
	{
		std::optional<typename FaceExtractor::Output> face = faceExtractor(std::forward<Input>(input));
		//// TEST!
		//dlib::array2d<dlib::rgb_pixel> &arr = *face;
		////dlib::matrix<dlib::rgb_pixel>& arr = *face;
		////Network::Input& arr = *face;
		//Network::Input mat(arr);
		//return network(mat);
		return face ? network(*face) : std::nullopt;
	}

private:
	FaceExtractor faceExtractor;
	Network network;
};	// DnnFaceDescriptorComputer


/*
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
	//static const inline dlib::pyramid_down<2> pyrDown;		// TODO: common
};	// DnnFaceDescriptorComputer
*/



#include "dnnfacedescriptorcomputer_impl.h"



#endif	//DNNDESCRIPTORCOMPUTER_H