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

    template <typename T, long NR, long NC, typename MM, typename L>
    struct hash<dlib::matrix<T, NR, NC, MM, L>>
    {
        /*
        * The rules for extending namespace std says:
        * 
        * It is allowed to add template specializations for any standard library class template to the namespace std only if the declaration 
        * depends on at least one program - defined type and the specialization satisfies all requirements for the original template, except 
        * where such specializations are prohibited.
        */                
        std::size_t operator()(const dlib::matrix<T, NR, NC, MM, L>& m) const noexcept  // the original version is noexcept
        {
            try
            {
                return dlib::hash(m);   // dlib::hash is not declared noexcept
            }
            catch (...)
            {
                return 0;
            }
        }
    };  // hash<dlib::matrix>

}  // std



#endif	// DLIBMATRIXHASH_H
