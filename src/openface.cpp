//#include "openfacedescriptor.h"
#include "openface.h"

#include <opencv2/core.hpp>

// TEST!
#include <iostream>		
#include <opencv2/highgui.hpp>


std::optional<OpenFace::OutputLabel> OpenFace::operator()(const cv::Mat& input, bool swapRB)
{
	CV_Assert(!input.empty());
	CV_Assert(input.type() == CV_32FC3 || input.type() == CV_8UC3);

	double scaleFactor = input.type() == CV_32FC3 ? 1.0 : 1 / 255.0;
	auto blob = cv::dnn::blobFromImage(input, scaleFactor, cv::Size(96, 96), cv::Scalar(0, 0, 0), swapRB, false, CV_32F);
	net.setInput(blob);	
	return net.forward().clone();	// it seems like a non-owning Mat is returned
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