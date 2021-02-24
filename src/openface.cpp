//#include "openfacedescriptor.h"
#include "openface.h"

#include <opencv2/core.hpp>

std::optional<OpenFace::OutputLabel> OpenFace::operator()(const cv::Mat& input, bool swapRB)
{
	CV_Assert(!input.empty());
	CV_Assert(input.type() == CV_32FC3 || input.type() == CV_8UC3);

	double scaleFactor = input.type() == CV_32FC3 ? 1.0 : 1 / 255.0;
	auto blob = cv::dnn::blobFromImage(input, scaleFactor, cv::Size(96, 96), cv::Scalar(0, 0, 0), swapRB, false, CV_32F);
	net.setInput(blob);
	return net.forward();
}


double operator - (const OpenFace::Descriptor& d1, const OpenFace::Descriptor& d2)
{
	//static_assert(std::is_same_v<OpenFace::OutputLabel, cv::Mat>, "The implementation assumes that OpenFace::OutputLabel is cv::Mat.");
	return cv::norm(d1.data - d2.data, cv::NORM_L2);	
}

std::istream& operator >> (std::istream& stream, OpenFace::Descriptor& descriptor)
{
	int type = -1;
	int rows = 0;
	stream >> type >> rows;

	CV_Assert(type == CV_32FC1 && rows > 0);
	cv::Mat mat(rows, 1, type);

	for (int i = 0; i < rows; ++i)
	{
		auto &elem = mat.at<double>(i);
		stream >> elem;
	}

	return stream;
}


std::ostream& operator << (std::ostream& stream, const OpenFace::Descriptor& descriptor)
{
	const cv::Mat& m = descriptor.data;
	
	CV_Assert(!m.empty());
	CV_Assert(m.dims == 2 && m.cols == 1);
	CV_Assert(m.type() == CV_32FC1);

	stream << m.type() << " " << m.rows << std::endl;

	for (int i = 0; i < m.rows; ++i)
	{
		stream << m.at<double>(i) << std::endl;
	}

	stream << std::endl;
	return stream;
}