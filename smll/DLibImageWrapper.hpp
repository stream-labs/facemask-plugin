// Copyright (C) 2009  Davis E. King (davis@dlib.net)
// License: Boost Software License   See LICENSE.txt for the full license.
#ifndef DLIB_IMAGE_WRAPPER_H_
#define DLIB_IMAGE_WRAPPER_H_

#pragma warning( push )
#pragma warning( disable: 4127 )
#pragma warning( disable: 4201 )
#pragma warning( disable: 4456 )
#pragma warning( disable: 4458 )
#pragma warning( disable: 4459 )
#pragma warning( disable: 4505 )
#pragma warning( disable: 4267 )
#pragma warning( disable: 4100 )
#include <dlib/algs.h>
#include <dlib/pixel.h>
#include <dlib/image_processing/generic_image.h>
#pragma warning( pop )


// ---------------------------------------------------------------------------
// This code copied and modified from dlib's internal image wrapper
//
// - use this to wrap arbitrary memory buffers to pass to dlib methods
// ---------------------------------------------------------------------------
namespace dlib
{
    
    template <
    typename pixel_type
    >
    class dlib_image_wrapper
    {
    public:
        typedef pixel_type type;
        typedef default_memory_manager mem_manager_type;
        
        dlib_image_wrapper (char* d, int w, int h, int s)
        {
            _data = d;
            _widthStep = s;
            _nr = h;
            _nc = w;
        }
        
        dlib_image_wrapper() : _data(0), _widthStep(0), _nr(0), _nc(0) {}
        
        unsigned long size () const { return static_cast<unsigned long>(_nr*_nc); }
        
        inline pixel_type* operator[](const long row )
        {
            // make sure requires clause is not broken
            DLIB_ASSERT(0 <= row && row < nr(),
                        "\tpixel_type* dlib_image_wrapper::operator[](row)"
                        << "\n\t you have asked for an out of bounds row "
                        << "\n\t row:  " << row
                        << "\n\t nr(): " << nr()
                        << "\n\t this:  " << this
                        );
            
            return reinterpret_cast<pixel_type*>( _data + _widthStep*row);
        }
        
        inline const pixel_type* operator[](const long row ) const
        {
            // make sure requires clause is not broken
            DLIB_ASSERT(0 <= row && row < nr(),
                        "\tconst pixel_type* dlib_image_wrapper::operator[](row)"
                        << "\n\t you have asked for an out of bounds row "
                        << "\n\t row:  " << row
                        << "\n\t nr(): " << nr()
                        << "\n\t this:  " << this
                        );
            
            return reinterpret_cast<const pixel_type*>( _data + _widthStep*row);
        }
        
        long nr() const { return _nr; }
        long nc() const { return _nc; }
        long width_step() const { return _widthStep; }
        
        dlib_image_wrapper& operator=( const dlib_image_wrapper& item)
        {
            _data = item._data;
            _widthStep = item._widthStep;
            _nr = item._nr;
            _nc = item._nc;
            return *this;
        }
        
    private:
        
        char* _data;
        long _widthStep;
        long _nr;
        long _nc;
    };
    
    // ----------------------------------------------------------------------------------------
    
    template <
    typename T
    >
    const matrix_op<op_array2d_to_mat<dlib_image_wrapper<T> > > mat (
                                                           const dlib_image_wrapper<T>& m
                                                           )
    {
        typedef op_array2d_to_mat<dlib_image_wrapper<T> > op;
        return matrix_op<op>(op(m));
    }
    
    // ----------------------------------------------------------------------------------------
    
    // Define the global functions that make dlib_image_wrapper a proper "generic image" according to
    // ../image_processing/generic_image.h
    template <typename T>
    struct image_traits<dlib_image_wrapper<T> >
    {
        typedef T pixel_type;
    };
    
    template <typename T>
    inline long num_rows( const dlib_image_wrapper<T>& img) { return img.nr(); }
    template <typename T>
    inline long num_columns( const dlib_image_wrapper<T>& img) { return img.nc(); }
    
    template <typename T>
    inline void* image_data(
                            dlib_image_wrapper<T>& img
                            )
    {
        if (img.size() != 0)
            return &img[0][0];
        else
            return 0;
    }
    
    template <typename T>
    inline const void* image_data(
                                  const dlib_image_wrapper<T>& img
                                  )
    {
        if (img.size() != 0)
            return &img[0][0];
        else
            return 0;
    }
    
    template <typename T>
    inline long width_step(
                           const dlib_image_wrapper<T>& img
                           ) 
    { 
        return img.width_step(); 
    }
    
    // ----------------------------------------------------------------------------------------
    
}

#endif // DLIB_IMAGE_WRAPPER_H_

