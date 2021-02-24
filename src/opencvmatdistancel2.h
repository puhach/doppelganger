#ifndef OPENCVMATDISTANCEL2_H
#define OPENCVMATDISTANCEL2_H

#include <opencv2/core.hpp>

template <typename T>
struct L2Distance;

template <>
struct L2Distance<cv::Mat>
{
	double operator()(const cv::Mat& m1, const cv::Mat& m2) const
	{
		return cv::norm(m1 - m2, cv::NORM_L2);
	}
};	// L2Distance


#endif	// OPENCVMATDISTANCEL2_H
