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

#include "applycolormap.hpp"
#include "afcolormap.hpp"
#include "cvcolormap.hpp"
#include <arrayfire.h>
#include <algorithm>

namespace advision {

    static const std::vector< float > __levels { 0.0, 0.2, 0.4, 0.6, 0.8, 0.97, 1.0 };

                                            // black, navy, cyan, green,yellow,red, white
    static const std::vector< float > __colors { 0.0, 0.0,  0.0,  0.0,  1.0,  1.0, 1.0  // R
                                               , 0.0, 0.0,  1.0,  1.0,  1.0,  0.0, 1.0  // G
                                               , 0.0, 0.5,  1.0,  0.0,  0.0,  0.0, 1.0  // B
    };

    // static constexpr size_t __nLevels = sizeof( __levels ) / sizeof( __levels[ 0 ] );

    // software colormap
    struct cvColor {
        const size_t nlevels;
        const std::vector< float > levels_;
        const std::vector< float > colors_;

        struct Color {
            Color( float r, float g, float b ) : red(r), green(g), blue(b) {}
            float red, green, blue;
        };

        cvColor( const std::vector< float >& levels
                 , const std::vector< float >& colors ) : nlevels( levels.size() )
                                                        , levels_( levels )
                                                        , colors_( colors ) {
        }

        inline float R( size_t level ) const { return colors_[ level ]; }
        inline float G( size_t level ) const { return colors_[ level + nlevels ]; }
        inline float B( size_t level ) const { return colors_[ level + nlevels * 2 ]; }

        Color operator ()( float value ) const  {

            size_t level = 0;
            while ( level < nlevels ) {
                if ( value < levels_[ level ] )
                    break;
                ++level;
            }

            if ( level >= nlevels )
                return Color( R( nlevels - 1 ), G( nlevels - 1 ), B( nlevels - 1 ) );

            if ( level == 0 )
                return Color( R( 0 ), G( 0 ), B( 0 ) );

            auto prev = level - 1;
            float frac = ( value - levels_[ prev ] ) / ( levels_[ level ] - levels_[ prev ] );

            return Color( ( ( R( level ) - R( prev ) ) * frac + R( prev ) )
                          , ( ( G( level ) - G( prev ) ) * frac + G( prev ) )
                          , ( ( B( level ) - B( prev ) ) * frac + B( prev ) ) );
        }
    };

#if HAVE_ARRAYFIRE && HAVE_CUDA
    namespace gpu_af {
        class ColorMap : cuda::afColorMap {
            // af::array levels_;
            // af::array colors_;
        public:
            ColorMap( const std::vector< float >& levels
                      , const std::vector< float >& colors )
                : cuda::afColorMap( af::array( levels.size(), 1, levels.data() )
                                    , af::array( 3, levels.size(), colors.data() ) ) {
            }
            
            ~ColorMap() {
            }
            
            //-------
            inline af::array apply( const af::array& gray ) const {
                return (*this)( gray ); // afColorMap( gray, levels_, colors_ );
            }
        };
    }
#endif
#if HAVE_OPENCV && HAVE_CUDA
    namespace gpu_cv {
        class ColorMap : cuda::cvColorMap {
        public:
            ColorMap( const std::vector< float >& levels
                      , const std::vector< float >& colors )
                : cuda::cvColorMap( levels, colors ) {
            }
            
            ~ColorMap() {
            }
            
            //-------
            inline cv::Mat apply( const cv::Mat& gray ) const {
                return (*this)( gray );
            }
        };
    }
#endif

    namespace cpu {
        class ColorMap {
            cvColor color_;
        public:
            ColorMap( const std::vector< float >& levels
                      , const std::vector< float >& colors ) : color_( levels, colors ) {
            }
            //
            inline cv::Mat apply( const cv::Mat& gray, double scaleFactor ) const {

                cv::Mat x( gray.rows, gray.cols, CV_8UC3 );
                
                for ( size_t i = 0; i < gray.rows; ++i ) {
                    for ( size_t j = 0; j < gray.cols; ++j ) {
                        double v = gray.at< float >( i, j ) * scaleFactor;
                        auto c = std::move( color_( v ) );
                        x.at< cv::Vec3b >( i, j )[ 0 ] = c.blue * 255;
                        x.at< cv::Vec3b >( i, j )[ 1 ] = c.green * 255;
                        x.at< cv::Vec3b >( i, j )[ 2 ] = c.red * 255;
                    }
                }
                return x;
            }
            //--
        };
        
    }
}


using namespace advision;

ApplyColorMap::~ApplyColorMap()
{
    std::cerr << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;        
}

ApplyColorMap::ApplyColorMap( size_t nlevels, const float * levels, const float * colors )
    : levels_( levels, levels + nlevels )
    , colors_( colors, colors + nlevels * 3 )
#if HAVE_OPENCV
    , cpu_( std::make_unique< cpu::ColorMap >( levels_, colors_ ) )
#endif
#if HAVE_CUDA && HAVE_ARRAYFIRE
    , gpu_af_( std::make_unique< gpu_af::ColorMap >( levels_, colors_ ) )
#endif
{
}

ApplyColorMap::ApplyColorMap()
    : levels_( __levels )
    , colors_( __colors )
#if HAVE_OPENCV
    , cpu_( std::make_unique< cpu::ColorMap >( levels_, colors_ ) )
#endif
#if HAVE_CUDA && HAVE_ARRAYFIRE
    , gpu_af_( std::make_unique< gpu_af::ColorMap >( levels_, colors_ ) )
#endif                                 
{
}

// must be grayscale 
cv::Mat
ApplyColorMap::operator()( const cv::Mat& mat, float scaleFactor, cuda_algo algo )
{
    if ( mat.type() != CV_32F )
        return cv::Mat();

#if HAVE_CUDA
    if ( algo == cuda_direct ) {
        return gpu_cv::ColorMap( levels_, colors_ ).apply( mat * scaleFactor );
    }
# if HAVE_ARRAYFIRE
    if ( algo == cuda_arrayfire && gpu_af_ ) {
        // f32 array
        af::array gray = af::array( mat.cols, mat.rows, 1, mat.ptr< float >( 0 ) ).T() * scaleFactor;

        // apply colorMap in CUDA kernel
        // af::array rgb = gpu::ColorMap( levels_, colors_ ).apply( gray );
        af::array rgb = gpu_af_->apply( gray );

        // make row major, rgb
        af::array cv_format_rgb = af::reorder( rgb.T(), 2, 0, 1 );

        // convert to cv::Mat (RGB)
        auto mat8u = cv::Mat( mat.rows, mat.cols, CV_8UC(3) );
        cv_format_rgb.as( u8 ).host( mat8u.ptr< uchar >( 0 ) );
        return mat8u;        
    }
# endif
#endif
    // software color mapping
    if ( algo == cuda_none )
        return cpu::ColorMap( levels_, colors_ ).apply( mat, scaleFactor );

    return cv::Mat();
}

#if HAVE_ARRAYFIRE && HAVE_CUDA
af::array
ApplyColorMap::operator()( const af::array& gray, float scaleFactor, bool gpu ) // must be grayscale 
{
    return gpu_af::ColorMap( levels_, colors_ ).apply( gray * scaleFactor );
}
#endif
