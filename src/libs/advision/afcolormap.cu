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
#include <arrayfire.h>
#include <af/cuda.h>

namespace advision { namespace af {

        enum RGB { Red = 0, Green = 1, Blue = 2 };
    
        template<typename T>
        struct afColor {
            const size_t nlevels_;
            const T * colors_;

            template< typename U > struct Color {
                __device__ Color( U r, U g, U b ) : red(r), green(g), blue(b) {}
                U red, green, blue;
            };
            
            __device__ afColor( size_t num, const T* rgb ) : nlevels_( num ), colors_(rgb) {
            }

            __device__ inline T R( int level ) const { return colors_[ level ]; }
            __device__ inline T G( int level ) const { return colors_[ level + nlevels_ ]; }
            __device__ inline T B( int level ) const { return colors_[ level + nlevels_ * 2 ]; }
            
            __device__ inline Color< T > operator ()( size_t level, float frac ) const {
                if ( level >= nlevels_ )
                    return Color< T > ( R( nlevels_ - 1 ), G( nlevels_ - 1 ), B( nlevels_ - 1 ) );
                if ( level == 0 )
                    return Color< T > ( R( 0 ), G( 0 ), B( 0 ) );
                auto prev = level - 1;
                return Color< T > ( ( ( R( level ) - R( prev ) ) * frac + R( prev ) )
                                    , ( ( G( level ) - G( prev ) ) * frac + G( prev ) )
                                    , ( ( B( level ) - B( prev ) ) * frac + B( prev ) ) );
            }
        };

    } // namespace af
} // namespace advision


__global__
void
af_colormap_kernel( const int num, const float * d_x, uint8_t * d_y
                    , const int nlevels, const float * d_levels, const float * d_colors )
{
    const int id = blockIdx.x * blockDim.x + threadIdx.x;

    if ( id < num ) {

        using namespace advision::af;

        afColor<float> afColor( nlevels, d_colors );    
        
        float r( afColor.R(0) ), g( afColor.G(0) ), b( afColor.B( 0 ) );
        float frac(0);
        
        int level = 0;

        while ( level < nlevels ) {
            if ( d_x[ id ] < d_levels[ level ] )
                break;
            ++level;
        }
        if ( level > 0 )
            frac = ( d_x[ id ] - d_levels[ level - 1 ] ) / ( d_levels[ level ] - d_levels[ level - 1 ] );

        auto c = afColor( level, frac );
        
        d_y[id + num * 0] = c.blue * 255;
        d_y[id + num * 1] = c.green * 255;
        d_y[id + num * 2] = c.red * 255;
    }
}

af::array
afColorMap( const af::array& gray, const af::array& levels, const af::array& colors )
{
    // Ensure any JIT kernels have executed
    gray.eval();
    levels.eval();
    colors.eval();

    // Determine ArrayFire's CUDA stream
    int cuda_id = afcu::getNativeId( af::getDevice() );
    cudaStream_t af_cuda_stream = afcu::getStream( cuda_id );

    const int num = gray.dims(0) * gray.dims(1);

    const float * d_gray = gray.device< float >();

    using advision::af_type_value;

    // result array
    af::array rgb = af::constant< uint8_t >( 0, gray.dims(0), gray.dims(1), 3, af_type_value< uint8_t >::value );

    uint8_t * d_rgb = rgb.device< uint8_t >();
    const float * d_levels = levels.device< float >();
    const float * d_colors = colors.device< float >();

    const int threads = 256;
    const int blocks = (num / threads) + ((num % threads) ? 1 : 0 );

    af_colormap_kernel <<< blocks, threads, 0, af_cuda_stream >>> ( num, d_gray, d_rgb, levels.dims(0), d_levels, d_colors );

    cudaDeviceSynchronize();

    gray.unlock();
    rgb.unlock();    
    levels.unlock();
    colors.unlock();

    return rgb;
}

