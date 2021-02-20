#ifndef DLIBMATRIXHASH_H
#define DLIBMATRIXHASH_H

#include <functional>

#include <dlib/matrix.h>


/*
* We need to define a hash function for the face descriptor type to be able to use it in an unordered_map.
* Custom specialization of std::hash can be injected in namespace std. It is also possible to define this 
* functor locally and add to the template argument list of an unordered_map, but since FaceDb makes no assumption
* on what Descriptor type is, we don't really know how to do it there.
*/

namespace std
{
    //template<> struct hash<MyClass>
    //{
    //    std::size_t operator()(const MyClass& mat) const noexcept
    //    {
    //        return 0;
    //    }
    //};
    
    template<> struct hash<dlib::rgb_pixel>
    {
        std::size_t operator()(const dlib::rgb_pixel& rgb) const noexcept
        {
            return (rgb.red << 16) | (rgb.green << 8) | rgb.blue;
        }
    };

    /*template<> struct hash<dlib::matrix<dlib::rgb_pixel>>
    {
        std::size_t operator()(const dlib::matrix<dlib::rgb_pixel>& mat) const noexcept
        {
            static std::hash<dlib::rgb_pixel> pixelHash;
            return mat.begin() == mat.end() ? 0 : (static_cast<std::size_t>(mat.size())<<25) ^ (pixelHash(mat(0))<<1) ^ pixelHash(mat(mat.size()-1));
        }
    };*/

    template<typename T, long nrows, long ncols> struct hash<dlib::matrix<T, nrows, ncols>>
    {
        static_assert(std::is_constructible_v<std::hash<T>>, "No standard hash function defined for this type.");

        std::size_t operator()(const dlib::matrix<T, nrows, ncols>& m) const noexcept
        {
            static std::hash<T> h;
            return m.begin() == m.end() ? 0 : static_cast<std::size_t>(m.size()) ^ (h(m(0)) << 1) ^ h(m(m.size() - 1));
        }
    };

    /*template<long nrows, long ncols> struct hash<dlib::matrix<float, nrows, ncols>>
    {
        std::size_t operator()(const dlib::matrix<float, nrows, ncols>& m) const noexcept
        {
            static std::hash<float> floatHash;
            return m.begin() == m.end() ? 0 : static_cast<std::size_t>(m.size()) ^ (floatHash(m(0))<<1) ^ floatHash(m(m.size()-1));
        }
    };*/
}  // std



#endif	// DLIBMATRIXHASH_H
