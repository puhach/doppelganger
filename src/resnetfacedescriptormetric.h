#ifndef RESNETFACEDESCRIPTORMETRIC_H
#define RESNETFACEDESCRIPTORMETRIC_H

#include "resnet.h"

#include "dlibmatrixdistancel2.h"
//#include "dlibmatrixl2distance.h"
//#include "l2distance.h"
//#include "l2distancedlibmatrix.h"

//static_assert(std::is_same<typename ResNet::OutputLabel, dlib::matrix<>>);
//using ResNetFaceDescriptorMetric = L2Distance<typename ResNet::OutputLabel>;
using ResNetFaceDescriptorMetric = L2Distance<typename ResNet::Descriptor>;

//template <typename T>
//struct ResMetric;
//
//template <typename T, long NR, long NC, typename MM, typename L>
//struct ResMetric<dlib::matrix<T, NR, NC, MM, L>>
//{
//	double operator()(const dlib::matrix<T, NR, NC, MM, L>& m1, const dlib::matrix<T, NR, NC, MM, L>& m2) const
//	{
//		return dlib::length(m1 - m2);
//	}
//};	// L2Distance



#endif	// RESNETFACEDESCRIPTORMETRIC_H