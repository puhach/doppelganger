#include "openface.h"

#include <opencv2/core.hpp>
#include <opencv2/dnn/dnn.hpp>



std::optional<OpenFace::Descriptor> OpenFace::operator()(const Input& input)
{
	CV_Assert(!input.empty());
	CV_Assert(input.type() == CV_32FC3 || input.type() == CV_8UC3);

	auto blob = cv::dnn::blobFromImage(input, 1 / 255.0, cv::Size(inputSize, inputSize), cv::Scalar(0, 0, 0), this->swapRB, false, CV_32F);
	net.setInput(blob);	
	return net.forward().clone();	// it seems like a non-owning Mat is returned
}


double operator - (const OpenFace::Descriptor& d1, const OpenFace::Descriptor& d2)
{
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
		stream << m.at<float>(0, i) << std::endl;
	}

	stream << std::endl;
	return stream;
}