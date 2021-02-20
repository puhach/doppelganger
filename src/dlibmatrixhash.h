#ifndef DLIBMATRIXHASH_H
#define DLIBMATRIXHASH_H

#include <functional>

#include <dlib/matrix.h>

#include "myclass.h"    // TEST!

/*
* We need to define a hash function for the face descriptor type to be able to use it in an unordered_map.
* Custom specialization of std::hash can be injected in namespace std. It is also possible to define this 
* functor locally and add to the template argument list of an unordered_map, but since FaceDb makes no assumption
* on what Descriptor type is, we don't really know how to do it there.
*/

namespace std
{
    template<> struct hash<MyClass>
    {
        std::size_t operator()(const MyClass& mat) const noexcept
        {
            return 0;
        }
    };
    
    template<> struct hash<dlib::matrix<dlib::rgb_pixel>>
    {
        std::size_t operator()(const dlib::matrix<dlib::rgb_pixel>& mat) const noexcept
        {
            //return mat.begin() == mat.end() ? 0 : mat.size() ^ std::hash<dlib::rgb_pixel>(mat(0));
            return mat.begin() == mat.end() ? 0 : mat.size() ^ 55;
            //std::size_t h1 = std::hash<std::string>{}(s.first_name);
            //std::size_t h2 = std::hash<std::string>{}(s.last_name);
            //return h1 ^ (h2 << 1); // or use boost::hash_combine
        }
    };
    
}  // std



#endif	// DLIBMATRIXHASH_H
