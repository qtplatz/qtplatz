/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "aftypes.hpp"
#include <stdio.h>

#include <thrust/device_vector.h>
#include <thrust/reduce.h>
#include <thrust/sequence.h>
#include <arrayfire.h>
#include <af/cuda.h>

typedef uint8_t rgb_type;

namespace af_extension {

    enum RGB { Red = 0, Green = 1, Blue = 2 };
    
    template<typename T>
    struct ColorTable {
        const int num_;
        const T * rgb_;

        __device__ ColorTable( int num, const T* rgb ) : num_( num ), rgb_(rgb) {
        }

        __device__ inline T color( int level, RGB code ) const {
            return rgb_[ level + num_ * code ];
        }

        __device__ T operator ()( int level, float frac, RGB code ) const {
            if ( level == 0 )
                return color( level, code );
            else if ( level == num_ )
                return color( num_ - 1, code );
            else if ( level > 0 )
                return ( color( level, code ) - color ( level - 1, code ) ) * frac + color( level - 1, code );
            else
                return T(0);
        }        
    };
}

__global__
void
colormap_kernel( const int num, const float * d_x, rgb_type * d_y
                 , const int nlevels, const float * d_levels, const float * d_colors )
{
    const int id = blockIdx.x * blockDim.x + threadIdx.x;

    float r(0), g(0), b(0), frac(0);
    int level = 0;

    af_extension::ColorTable<float> table( nlevels, d_colors );

    if ( id < num ) {
        while ( level < nlevels ) {
            if ( d_x[ id ] < d_levels[ level ] )
                break;
            ++level;
        }
        if ( level > 0 ) {
            frac = ( d_x[ id ] - d_levels[ level - 1 ] ) / ( d_levels[ level ] - d_levels[ level - 1 ] );
            r = table( level, frac, af_extension::Red  );
            g = table( level, frac, af_extension::Green );
            b = table( level, frac, af_extension::Blue );
        }

        d_y[id + num * 0] = r * 255;
        d_y[id + num * 1] = g * 255;
        d_y[id + num * 2] = b * 255;
    }
}

af::array
colorMap( const af::array& gray, const af::array& levels, const af::array& colors )
{
    gray.eval(); // Ensure any JIT kernels have executed
    levels.eval();
    colors.eval();
    int cuda_id = afcu::getNativeId( af::getDevice() ); // Determine ArrayFire's CUDA stream
    cudaStream_t af_cuda_stream = afcu::getStream( cuda_id );

    const int num = gray.dims(0) * gray.dims(1);

    const float * d_gray = gray.device< float >();

    using namespace arrayfire;
    
    // result array

    af::array rgb = af::constant< rgb_type >( 0, gray.dims(0), gray.dims(1), 3, af_type_value< rgb_type >::value );
    rgb_type * d_rgb = rgb.device< rgb_type >();

    const float * d_levels = levels.device< float >();
    const float * d_colors = colors.device< float >();

    const int threads = 256;
    const int blocks = (num / threads) + ((num % threads) ? 1 : 0 );

    colormap_kernel <<< blocks, threads, 0, af_cuda_stream >>> ( num, d_gray, d_rgb, levels.dims(0), d_levels, d_colors );

    cudaDeviceSynchronize();

    rgb.unlock();

    return rgb;
}

