#ifndef NETWORKTRAITS_H
#define NETWORKTRAITS_H

namespace network_traits
{
	// As a constexpr variable must be initialized, we prevent it from being incorrectly used with types other than defined network types
	template <typename >
	constexpr inline unsigned long inputImageSize;
}

#endif	// NETWORKTRAITS_H