//#include "openfacedescriptor.h"
#include "openface.h"

#include <opencv2/core.hpp>
#include <opencv2/dnn/dnn.hpp>

// TEST!
#include <iostream>		
#include <opencv2/highgui.hpp>


//OpenFace& OpenFace::operator = (const OpenFace& other)
//{
//	this->modelPath = other.modelPath;
//	this->net = cv::dnn::readNetFromTorch(modelPath);	// there seems to be no other way to make a deep copy of cv::dnn::Net
//	return *this;
//}

std::optional<OpenFace::OutputLabel> OpenFace::operator()(const cv::Mat& input, bool swapRB)
{
	CV_Assert(!input.empty());
	CV_Assert(input.type() == CV_32FC3 || input.type() == CV_8UC3);

	// TODO: define input size member constant / type trait
	double scaleFactor = input.type() == CV_32FC3 ? 1.0 : 1 / 255.0;
	auto blob = cv::dnn::blobFromImage(input, scaleFactor, cv::Size(96, 96), cv::Scalar(0, 0, 0), swapRB, false, CV_32F);
	net.setInput(blob);	
	return net.forward().clone();	// it seems like a non-owning Mat is returned
}

//std::vector<std::optional<OpenFace::OutputLabel>> OpenFace::operator()(const std::vector<std::optional<cv::Mat>>& inputs, bool swapRB)
std::vector<OpenFace::OutputLabel> OpenFace::operator()(const std::vector<cv::Mat>& inputs, bool swapRB)
{
	// TODO: scale factor must be consistent with a single argument version
	auto inBlob = cv::dnn::blobFromImages(inputs, 1 / 255.0, cv::Size(96, 96), cv::Scalar(0, 0, 0), swapRB, false, CV_32F);
	net.setInput(inBlob);
	//std::vector<cv::Mat> outputBlobs;
	//net.forward(outputBlobs);
	auto outBlob = net.forward();

	//std::cout << "Output blob size: " << outBlob.rows << std::endl;
		
	std::vector<OpenFace::OutputLabel> outputs{ inputs.size()};
	for (int i = 0; i < outBlob.rows; ++i)
	{
		outputs[i] = outBlob.row(i).clone(); // TODO: batch version seems to work without clone()
	}

	return outputs;
}

double operator - (const OpenFace::Descriptor& d1, const OpenFace::Descriptor& d2)
{
	//static_assert(std::is_same_v<OpenFace::OutputLabel, cv::Mat>, "The implementation assumes that OpenFace::OutputLabel is cv::Mat.");
	return cv::norm(d1.data - d2.data, cv::NORM_L2);	
}

std::istream& operator >> (std::istream& stream, OpenFace::Descriptor& descriptor)
{
	int type = -1;
	int cols = 0;
	stream >> type >> cols;

	CV_Assert(type == CV_32FC1 && cols > 0);
	cv::Mat mat(1, cols, type);

	for (int i = 0; i < cols; ++i)
	{
		auto &elem = mat.at<float>(0, i);
		stream >> elem;
	}

	descriptor.data = std::move(mat);

	return stream;
}


std::ostream& operator << (std::ostream& stream, const OpenFace::Descriptor& descriptor)
{
	const cv::Mat& m = descriptor.data;
	
	CV_Assert(!m.empty());
	CV_Assert(m.dims == 2 && m.rows == 1);
	CV_Assert(m.type() == CV_32FC1);

	stream << m.type() << " " << m.cols << std::endl;

	for (int i = 0; i < m.cols; ++i)
	{
		//stream << m.at<double>(i) << std::endl;
		stream << m.at<float>(0, i) << std::endl;
	}

	stream << std::endl;
	return stream;
}