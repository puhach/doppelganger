#ifndef OPENFACEDESCRIPTORMETRIC_H
#define OPENFACEDESCRIPTORMETRIC_H

#include "openface.h"
//#include "opencvmatdistancel2.h"
//#include "openfacedescriptor.h"	// TEST!

//using OpenFaceDescriptorMetric = L2Distance<typename OpenFace::OutputLabel>;

template <typename T>
struct L2Distance;

//// TEST!
//template <>
//struct L2Distance<OpenFaceDescriptor>
//{
//	double operator()(const OpenFaceDescriptor& d1, const OpenFaceDescriptor& d2) const
//	{
//		return d1 - d2;
//	}
//};	// L2Distance



template <>
struct L2Distance<OpenFace::Descriptor>
{
	double operator()(const OpenFace::Descriptor& d1, const OpenFace::Descriptor& d2) const
	{
		return d1 - d2;
	}
};	// L2Distance


#endif	// OPENFACEDESCERIPTORMETRIC_H
