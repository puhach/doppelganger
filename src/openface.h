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

	// There are multiple input types, hence we don't define an alias
	//using Input = cv::InputArray;
	//using OutputLabel = cv::Mat;
	//using OutputLabel = OpenFaceDescriptor;		// TODO: rename OutputLabel

	class Descriptor
	{
		friend double operator - (const Descriptor& d1, const Descriptor& d2);
		friend std::ostream& operator << (std::ostream& stream, const Descriptor& descriptor);
		friend std::istream& operator >> (std::istream& stream, Descriptor& descriptor);

		using DataType = cv::Mat;

	public:
		Descriptor() = default;

		Descriptor(const DataType& data)
			: data(data) {}

		Descriptor(DataType&& data)
			: data(std::move(data)) {}

		// TODO: define copy/move semantics

	private:
		//OpenFace::OutputLabel label;
		DataType data;
	};	// Descriptor

	using OutputLabel = Descriptor;		// TODO: rename OutputLabel


	OpenFace(const std::string& modelPath) 
		: modelPath(modelPath)
		, net(cv::dnn::readNetFromTorch(modelPath))
	{ }

	OpenFace(const OpenFace& other)
		: modelPath(other.modelPath)
		, net(cv::dnn::readNetFromTorch(modelPath))		// OpenCV provides no way to perform a deep copy of dnn::Net
	{ }

	OpenFace(OpenFace&& other) = default;

	OpenFace& operator = (const OpenFace& other)
	{
		this->modelPath = other.modelPath;
		this->net = cv::dnn::readNetFromTorch(modelPath);	// there seems to be no other way to make a deep copy of cv::dnn::Net
		return *this;
	}

	OpenFace& operator = (OpenFace&& other) = default;
	
	std::optional<OutputLabel> operator()(const cv::Mat& input, bool swapRB);


private:
	cv::String modelPath;
	cv::dnn::Net net;
};	// OpenFace

#endif	// OPENFACE_H
