#ifndef OPENFACE_H
#define OPENFACE_H

#include <optional>
#include <string>

#include <opencv2/dnn.hpp>



/*
* OpenFace implements a callable object to perform face recognition by means of the OpenFace model. 
* It takes in an image or a range of images and outputs face descriptors wrapped into std::optional<T>, which can be std::nullopt in 
* case of a failure. 
*/

class OpenFace
{
public:

	using Input = cv::Mat;

	class Descriptor	// Wraps the real output of the model into an object which can be serialized and compared for similarity
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
		DataType data;
	};	// Descriptor

	static constexpr unsigned long inputSize = 96;

	OpenFace(const std::string& modelPath, bool swapRB) 
		: net(cv::dnn::readNetFromTorch(modelPath))
		, swapRB(swapRB) { }
		
	OpenFace(const OpenFace& other) = delete;	// OpenCV provides no way to perform a deep copy of dnn::Net
	OpenFace(OpenFace&& other) = default;
	
	OpenFace& operator = (const OpenFace& other) = delete;	// OpenCV provides no way to perform a deep copy of dnn::Net
	OpenFace& operator = (OpenFace&& other) = default;
	
	std::optional<Descriptor> operator()(const Input& input);

	template <class InputIterator, class OutputIterator>
	OutputIterator operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead);	

private:
	cv::dnn::Net net;
	bool swapRB;
};	// OpenFace


template <class InputIterator, class OutputIterator>
OutputIterator OpenFace::operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead)
{
	if (inHead == inTail)
		return outHead;

	auto inBlob = cv::dnn::blobFromImages(std::vector<cv::Mat>(inHead, inTail), 1 / 255.0, cv::Size(inputSize, inputSize)
		, cv::Scalar(0, 0, 0), this->swapRB, false, CV_32F);
	net.setInput(inBlob);
	auto outBlob = net.forward();

	for (int i = 0; i < outBlob.rows; ++i)
	{
		*outHead = outBlob.row(i).clone();	// the network does not seem to give away data ownership and may delete/reuse it any time
		++outHead;
	}

	return outHead;
}	// operator ()


#endif	// OPENFACE_H
