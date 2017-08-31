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

#include "colormap.hpp"
#include "cvtypes.hpp"
#include <adportable/debug.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <QImage>

namespace cuda {

    namespace rgb {
        
        enum RGB { Red = 0, Green = 1, Blue = 2 };
    
        template<typename T>
        struct cvColor {
            const size_t nlevels_;
            const T * colors_;

            template< typename U > struct Color {
                __device__ Color( U r, U g, U b ) : red(r), green(g), blue(b) {}
                U red, green, blue;
            };
            
            __device__ cvColor( size_t num, const T* rgb ) : nlevels_( num ), colors_(rgb) {
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
    } // rgb

} // namespace cuda


template<typename T > __global__ void
colormap_kernel( const int num, const T * d_x, uint8_t * d_y
                 , const int nlevels, const float * d_levels, const float * d_colors
                 , float scaleFactor
                 , bool isBGR = false )
{
    const int id = blockIdx.x * blockDim.x + threadIdx.x;

    if ( id < num ) {

        using namespace cuda::rgb;

        cvColor<float> cvColor( nlevels, d_colors );    
        
        float r( cvColor.R(0) ), g( cvColor.G(0) ), b( cvColor.B( 0 ) );
        float frac(0);
        
        int level = 0;
        T dx = scaleFactor * d_x[ id ];

        while ( level < nlevels ) {
            if ( dx < d_levels[ level ] )
                break;
            ++level;
        }
        if ( level > 0 )
            frac = ( dx - d_levels[ level - 1 ] ) / ( d_levels[ level ] - d_levels[ level - 1 ] );

        auto c = cvColor( level, frac );

        if ( isBGR ) {
            d_y[(id * 3) + 0] = c.blue * 255;
            d_y[(id * 3) + 1] = c.green * 255;
            d_y[(id * 3) + 2] = c.red * 255;
        } else {
            d_y[(id * 3) + 0] = c.red * 255;
            d_y[(id * 3) + 1] = c.green * 255;
            d_y[(id * 3) + 2] = c.blue * 255;
        }
    }
}

namespace cuda {

    struct ColorMapHelper {

        const thrust::device_vector< float >& d_levels_;
        const thrust::device_vector< float >& d_colors_;

        ColorMapHelper( const thrust::device_vector< float >& levels
                        , const thrust::device_vector< float >& colors ) : d_levels_( levels )
                                                                         , d_colors_( colors ) {
        }

        // matrix<> -> apply colormap --> QImage
        template< typename T > QImage operator()( const boost::numeric::ublas::matrix< T >& m, double scaleFactor ) const {
            
            const int num = m.size1() * m.size2();
            const int threads = 64; // 1024; // 512; //256;
            const int blocks = (num / threads) + ((num % threads) ? 1 : 0 );
            
            //auto p_m = reinterpret_cast< const T * >( m.data() );
            auto p_m = m.data().begin();
            
            T * d_m(0);
            cudaMalloc( &d_m, num * sizeof( T ) );
            cudaMemcpyAsync( d_m, p_m, num * sizeof( T ), cudaMemcpyHostToDevice );
            
            uint8_t * d_rgb(0);
            cudaMalloc( &d_rgb, num * 3 * sizeof(uint8_t) );

            colormap_kernel <<< blocks, threads >>>
                ( num
                  , d_m
                  , d_rgb
                  , d_levels_.size()
                  , thrust::raw_pointer_cast( d_levels_.data() )
                  , thrust::raw_pointer_cast( d_colors_.data() )
                  , float( scaleFactor )
                  , false // isBGR ?
                    );
            //----------
            QImage rgb( m.size1(), m.size2(), QImage::Format_RGB888 );    

            cudaMemcpyAsync( rgb.bits(), d_rgb, num * 3 * sizeof(unsigned char), cudaMemcpyDeviceToHost );

            cudaFree( d_m );
            cudaFree( d_rgb );

            cudaStreamSynchronize( 0 );    
            return rgb;                
        }

        // cv::Mat -> apply color map --> QImage
        QImage operator()( const cv::Mat& gray, double scaleFactor ) const {
            const int num = gray.cols * gray.rows;
            const int threads = 64; // 1024; // 512; //256;
            const int blocks = (num / threads) + ((num % threads) ? 1 : 0 );
    
            const float * p_gray = reinterpret_cast< const float * >( gray.ptr() );

            //thrust::device_vector< float > d_gray( p_gray, p_gray + num );    
            float * d_gray(0);
            cudaMalloc( &d_gray, num * sizeof( float ) );
            cudaMemcpyAsync( d_gray, p_gray, num * sizeof( float ), cudaMemcpyHostToDevice );

            uint8_t * d_rgb(0);
            cudaMalloc( &d_rgb, num * 3 * sizeof(uint8_t) );

            colormap_kernel <<< blocks, threads >>>
                ( num
                  , d_gray
                  , d_rgb
                  , d_levels_.size()
                  , thrust::raw_pointer_cast( d_levels_.data() )
                  , thrust::raw_pointer_cast( d_colors_.data() )
                  , float( scaleFactor )                  
                  , true
                    );    
            //--
            QImage rgb( gray.rows, gray.cols, QImage::Format_RGB888 );    

            cudaMemcpyAsync( rgb.bits(), d_rgb, num * 3 * sizeof(unsigned char), cudaMemcpyDeviceToHost );

            cudaFree( d_gray );
            cudaFree( d_rgb );
            
            cudaStreamSynchronize( 0 );
            return rgb;
        }
    };
}

///////////////////////

cuda::ColorMap::ColorMap( const std::vector< float >& levels
                          , const std::vector< float >& colors )
    : d_levels_( levels.begin(), levels.end() )
    , d_colors_( colors.begin(), colors.end() )
{
}

cuda::ColorMap::~ColorMap()
{
}

template<>
QImage
cuda::ColorMap::operator()( const boost::numeric::ublas::matrix<float>& m, double scaleFactor ) const
{
    return cuda::ColorMapHelper( d_levels_, d_colors_ )( m, scaleFactor );
}

template<>
QImage
cuda::ColorMap::operator()( const boost::numeric::ublas::matrix<double>& m, double scaleFactor ) const
{
    return cuda::ColorMapHelper( d_levels_, d_colors_ )( m, scaleFactor );
}

#if HAVE_OPENCV
QImage
cuda::ColorMap::operator()( const cv::Mat& gray, double scaleFactor ) const
{
    return cuda::ColorMapHelper( d_levels_, d_colors_ )( gray, scaleFactor );
}
#endif
