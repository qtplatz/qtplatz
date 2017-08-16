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

    struct Color {
        float r; float g; float b; float v;
        Color( float _r = 0, float _g = 0, float _b = 0, float _v = 0 ) : r(_r), g(_g), b(_b), v(_v) {}
        float blue() const { return b; }
        float green() const { return g; }
        float red() const { return r; }
        float value() const { return v; }
    };

    __constant__ float __levels[] = { 0.0, 0.2, 0.4, 0.6, 0.8, 0.97, 1.0 }; // 7 steps

    class ColorMap {
        thrust::device_vector< Color > colors_;
    public:
        ColorMap() {
            colors_.push_back( Color( 0,     0, 0.0, 0.00 ) );
            colors_.push_back( Color( 0,     0, 0.5, 0.20 ) );
            colors_.push_back( Color( 0,   1.0, 1.0, 0.40 ) ); // cyan
            colors_.push_back( Color( 0,   1.0,   0, 0.60 ) ); // green
            colors_.push_back( Color( 1.0, 1.0,   0, 0.80 ) ); // yellow
            colors_.push_back( Color( 1.0,   0,   0, 0.97 ) ); // red
            colors_.push_back( Color( 1.0, 1.0, 1.0, 1.00 ) ); // white
        }


        __device__ const Color color( float value ) const {

            thrust::device_vector< Color > results(1);
            
            auto it
                = thrust::lower_bound( colors_.begin(), colors_.end()
                                       , &value, &value
                                       , results.begin()
                                       , []( const Color& c, const float& v )->bool{
                                           return c.value < v;
                                       } );
                                 
#if 0
            thrust::device_vector< Color > results(1);
            thrust::device_vector< float > values;
            values.push_back( value );

            ColorMap::const_iterator it
                = thrust::lower_bound( colors_.begin(), colors_.end()
                                       , values.begin(), values.end()
                                       , results.begin()
                                       , []( const Color& c, const float& v )->bool{
                                           return c.value < v;
                                       } );

            if ( it == colors_.end() )
                return Color( colors_.back() );
            
            if ( it == colors_.begin() )
                return Color( *it );

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
    static std::once_flag flag;
    std::call_once( flag, [](){ __colorMap__ = thrust::device_new< cuda::ColorMap >(); } );

    if ( src.type() != CV_32F )
        return;

    ADDEBUG() << "cudaApplycolormap";
    
    dst = cv::Mat( src.rows, src.cols, CV_8UC3 );

    for ( size_t i = 0; i < src.rows; ++i ) {
        for ( size_t j = 0; j < src.cols; ++j ) {
            float v = src.at< float >( i, j ) * scale;
            auto c = __colorMap__.color( v );
            dst.at< cv::Vec3b >( i, j )[ 0 ] = c.blue();
            dst.at< cv::Vec3b >( i, j )[ 1 ] = c.green();
            dst.at< cv::Vec3b >( i, j )[ 2 ] = c.red();
        }
    }
    return;
}

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

