#ifndef OPENFACEDESCRIPTORMETRIC_H
#define OPENFACEDESCRIPTORMETRIC_H

#include "openface.h"


/*
* L2Distance for OpenFace descriptors is defined in terms of the minus operator, which is overloaded to return the L2 distance value.
*/

template <typename T>
struct L2Distance;

template <>
struct L2Distance<OpenFace::Descriptor>
{
	double operator()(const OpenFace::Descriptor& d1, const OpenFace::Descriptor& d2) const
	{
		return d1 - d2;
	}
};	// L2Distance


#endif	// OPENFACEDESCERIPTORMETRIC_H
