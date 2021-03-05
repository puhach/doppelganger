#ifndef DLIBMATRIXDISTANCEL2
#define DLIBMATRIXDISTANCEL2


#include <dlib/matrix.h>

template <typename T>
struct L2Distance;

template <typename T, long NR, long NC, typename MM, typename L>
struct L2Distance<dlib::matrix<T, NR, NC, MM, L>>
{
	double operator()(const dlib::matrix<T, NR, NC, MM, L>& m1, const dlib::matrix<T, NR, NC, MM, L>& m2) const
	{
		return dlib::length(m1 - m2);
	}
};	// L2Distance


#endif	// DLIBMATRIXDISTANCEL2