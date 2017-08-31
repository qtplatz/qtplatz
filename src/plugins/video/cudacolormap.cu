/*
 * Copyright 1993-2015 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#include "cudacolormap.hpp"
#include <adportable/debug.hpp>
#include <opencv2/opencv.hpp>
#include <helper_cuda.h>
#include <math.h>

// CUDA standard includes
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>

#include <thrust/binary_search.h>
#include <thrust/device_vector.h>
#include <thrust/device_new.h>
#include <thrust/device_ptr.h>
#include <thrust/functional.h>
#include <thrust/reduce.h>
#include <thrust/sequence.h>
#include <memory>
#include <thread>
#include <mutex>

namespace cuda {

    template<typename T>
    struct Fun
    {
        __device__ T operator()(T t1, T t2)  {
            auto result = t1+t2;
            return result;
        }
    };

    int
    run()
    {
        const int N = 100;
        thrust::device_vector<int> vec(N);
        thrust::sequence(vec.begin(),vec.end());
        auto op = Fun<int>();
        return thrust::reduce(vec.begin(),vec.end(),0,op);
    }

    struct Color {
        float r; float g; float b; float v;
        __host__ __device__ Color( float _r = 0, float _g = 0, float _b = 0, float _v = 0 ) : r(_r), g(_g), b(_b), v(_v) {}
        __host__ __device__ float blue() const { return b; }
        __host__ __device__ float green() const { return g; }
        __host__ __device__ float red() const { return r; }
        __host__ __device__ float value() const { return v; }
    };

    template< typename T > struct grater_than {
        T value_;
        __host__ __device__ grater_than( T value ) : value_( value ) {}
        __host__ __device__ bool operator ()( const Color& a ) { return a.v > value_; }
    };

    class ColorMap {
        thrust::device_vector< Color > colors_;
    public:
        __host__ __device__ ColorMap() {
            colors_.push_back( Color( 0,     0, 0.0, 0.00 ) );
            colors_.push_back( Color( 0,     0, 0.5, 0.20 ) );
            colors_.push_back( Color( 0,   1.0, 1.0, 0.40 ) ); // cyan
            colors_.push_back( Color( 0,   1.0,   0, 0.60 ) ); // green
            colors_.push_back( Color( 1.0, 1.0,   0, 0.80 ) ); // yellow
            colors_.push_back( Color( 1.0,   0,   0, 0.97 ) ); // red
            colors_.push_back( Color( 1.0, 1.0, 1.0, 1.00 ) ); // white
        }

        __host__ __device__ const Color color( float value ) const {
#if 0
            auto it = thrust::find_if( colors_.begin(), colors_.end(), grater_than<float>( value ) );

            if ( it == colors_.end() )
                return Color( colors_.back() );
            
            if ( it == colors_.begin() )
                return Color( *it );
#endif

#if 0                
            ColorMap::const_iterator it
                = thrust::lower_bound( colors_.begin(), colors_.end()
                                       , values.begin(), values.end()
                                       , results.begin()
                                       , []( const Color& c, const float& v )->bool{
                                           return c.value < v;
                                       } );


            thrust::device_reference< const Color > ref = *it;

            auto prev = it - 1;
            //ADDEBUG() << results[0]->r; // << ", " << thrust::get< 0 >( *prev );

            ColorMap::const_iterator prev = it - 1;
            float frac = ( value - prev->value ) / ( it->value - prev->value );
            
            float r = ( it->r - prev->r ) * frac + prev->r;
            float g = ( it->g - prev->g ) * frac + prev->g;
            float b = ( it->b - prev->b ) * frac + prev->b;
            
            return Color( r, g, b );
#endif            
            return Color( 0, 0, 0, 0 );
        }
    };

}

using namespace cuda;

static thrust::device_ptr< cuda::ColorMap > __colorMap__;

void
cudaApplyColorMap( const cv::Mat& src, cv::Mat& dst, float scale )
{
#if 0
    static std::once_flag flag;
    std::call_once( flag, [](){ __colorMap__ = thrust::device_new< cuda::ColorMap >(); } );

    if ( src.type() != CV_32F )
        return;
    dst = cv::Mat( src.rows, src.cols, CV_8UC3 );

    auto rptr = thrust::raw_pointer_cast( __colorMap__ );

    for ( size_t i = 0; i < src.rows; ++i ) {
        for ( size_t j = 0; j < src.cols; ++j ) {
            float v = src.at< float >( i, j ) * scale;
            auto c = rptr->color( v );
            dst.at< cv::Vec3b >( i, j )[ 0 ] = c.blue();
            dst.at< cv::Vec3b >( i, j )[ 1 ] = c.green();
            dst.at< cv::Vec3b >( i, j )[ 2 ] = c.red();
        }
    }
#endif
    return;
}

