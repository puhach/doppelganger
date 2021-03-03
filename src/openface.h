#ifndef OPENFACE_H
#define OPENFACE_H

//#include "openfacedescriptor.h"
#include "networktraits.h"

#include <optional>
#include <string>

#include <opencv2/dnn.hpp>
//#include <opencv2/dnn/dnn.hpp>

//#include <dlib/matrix.h>
//#include <dlib/opencv.h>


/*
* OpenFace is not designed to be used in DnnFaceDescriptorComputer directly for the call operator
* requires the client code to specify whether the Red and Blue channels have to be swapped (OpenCV
* provides no way to distinguish an RGB image from a BGR image). That said, we can always wrap it
* in a new class taking the swapRB parameter as a constructor argument.
*/

class OpenFace
{
public:

	using Input = cv::Mat;

	class Descriptor
	{
		friend double operator - (const Descriptor& d1, const Descriptor& d2);
		friend std::ostream& operator << (std::ostream& stream, const Descriptor& descriptor);
		friend std::istream& operator >> (std::istream& stream, Descriptor& descriptor);

		using DataType = cv::Mat;	// the output type of the network
		
	public:
		Descriptor() = default;

		Descriptor(const DataType& data)
			: data(data) {}

		Descriptor(DataType&& data)
			: data(std::move(data)) {}

		Descriptor(const Descriptor& other)
			: data(other.data.clone()) { }

		Descriptor(Descriptor&& other) = default;

		Descriptor& operator = (const Descriptor& other)
		{
			other.data.copyTo(this->data);
			return *this;
		}
		
		Descriptor& operator = (Descriptor&& other) = default;

	private:
		//OpenFace::OutputLabel label;
		DataType data;
	};	// Descriptor

	//using OutputLabel = Descriptor;		// TODO: rename OutputLabel

	static constexpr unsigned long inputImageSize = 96;


	OpenFace(const std::string& modelPath, bool swapRB) 
		: net(cv::dnn::readNetFromTorch(modelPath))
		, swapRB(swapRB)
	{ }

	// OpenCV provides no way to perform a deep copy of dnn::Net
	OpenFace(const OpenFace& other) = delete;
	//OpenFace(const OpenFace& other)
	//	: modelPath(other.modelPath)
	//	, net(cv::dnn::readNetFromTorch(modelPath))		// OpenCV provides no way to perform a deep copy of dnn::Net
	//	, swapRB(other.swapRB)
	//{ }

	OpenFace(OpenFace&& other) = default;

	// OpenCV provides no way to perform a deep copy of dnn::Net
	OpenFace& operator = (const OpenFace& other) = delete;
	//{
	//	this->modelPath = other.modelPath;
	//	this->net = cv::dnn::readNetFromTorch(modelPath);	// there seems to be no other way to make a deep copy of cv::dnn::Net
	//	this->swapRB = other.swapRB;
	//	return *this;
	//}

	// TODO: add getter/setter for swapRB

	OpenFace& operator = (OpenFace&& other) = default;
	
	std::optional<Descriptor> operator()(const Input& input);
	//std::optional<OutputLabel> operator()(const Input& input);
	//std::optional<OutputLabel> operator()(const cv::Mat& input);

	///std::vector<std::optional<OutputLabel>> operator()(const std::vector<std::optional<cv::Mat>>& inputs, bool swapRB);
	//std::vector<OutputLabel> operator()(const std::vector<cv::Mat>& inputs);

	template <class InputIterator, class OutputIterator>
	OutputIterator operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead);	

private:
	//cv::String modelPath;
	cv::dnn::Net net;
	bool swapRB;
};	// OpenFace

#include "openface_impl.h"

#endif	// OPENFACE_H
