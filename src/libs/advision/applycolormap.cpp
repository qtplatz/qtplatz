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
#include <arrayfire.h>
#include <algorithm>

namespace advision {

    static const float __levels [] = { 0.0, 0.2, 0.4, 0.6, 0.8, 0.97, 1.0 };

                                    // black, navy, cyan, green,yellow,red, white
    static const float __colors [] = { 0.0,   0.0,  0.0,  0.0,  1.0,  1.0, 1.0      // R
                                       , 0.0, 0.0,  1.0,  1.0,  1.0,  0.0, 1.0    // G
                                       , 0.0, 0.5,  1.0,  0.0,  0.0,  0.0, 1.0  // B
    };

    static constexpr size_t __nLevels = sizeof( __levels ) / sizeof( __levels[ 0 ] );


    // software colormap
    struct cvColor {
        const size_t nlevels = __nLevels;
        const std::vector< float > levels_;
        const std::vector< float > colors_;

        struct Color {
            Color( float r, float g, float b ) : red(r), green(g), blue(b) {}
            float red, green, blue;
        };

        cvColor() : levels_( __levels, __levels + __nLevels )
                  , colors_( __colors, __colors + __nLevels * 3 ) {
        }

        inline float R( size_t level ) const { return colors_[ level ]; }
        inline float G( size_t level ) const { return colors_[ level + nlevels ]; }
        inline float B( size_t level ) const { return colors_[ level + nlevels * 2 ]; }

        Color operator ()( float value ) const  {
#if 0
            auto it = std::lower_bound( levels_.begin(), levels_.end(), value
                                        , []( const auto& l, const float& v ){ return l < v; } );

            size_t level = std::distance( levels_.begin(), it );
#else
            size_t level = 0;
            while ( level < nlevels ) {
                if ( value < levels_[ level ] )
                    break;
                ++level;
            }
#endif
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
}


using namespace advision;

ApplyColorMap::ApplyColorMap( size_t nlevels, const float * levels, const float * colors )
    : levels_( af::array( nlevels, 1, levels ) )
    , colors_( af::array( 3, nlevels, colors ) )
{
}

ApplyColorMap::ApplyColorMap()
    : levels_( af::array( sizeof( __levels )/sizeof( __levels[ 0 ] ), 1, __levels ) )
    , colors_( af::array( 3, sizeof( __levels )/sizeof( __levels[ 0 ] ), __colors ) )
{
}

cv::Mat
ApplyColorMap::operator()( const cv::Mat& mat, float scaleFactor, bool gpu ) // must be grayscale 
{
    if ( mat.type() != CV_32F )
        return cv::Mat();

#if HAVE_CUDA && HAVE_ARRAYFIRE
    if ( gpu ) {
        // f32 array
        af::array gray = af::array( mat.cols, mat.rows, 1, mat.ptr< float >( 0 ) ).T() * scaleFactor;

        // apply colorMap in CUDA kernel
        af::array rgb = afColorMap( gray, levels_, colors_ ); // return u8 array

        // make row major, rgb
        af::array cv_format_rgb = af::reorder( rgb.T(), 2, 0, 1 );

        // convert to cv::Mat (RGB)
        auto mat8u = cv::Mat( mat.rows, mat.cols, CV_8UC(3) );
        cv_format_rgb.as( u8 ).host( mat8u.ptr< uchar >( 0 ) );
        return mat8u;        
    }
#endif

    // software color mapping
    cvColor color;
    cv::Mat x( mat.rows, mat.cols, CV_8UC3 );
    for ( size_t i = 0; i < mat.rows; ++i ) {
        for ( size_t j = 0; j < mat.cols; ++j ) {
            double v = mat.at< float >( i, j ) * scaleFactor;
            auto c = color( v );
            x.at< cv::Vec3b >( i, j )[ 0 ] = c.blue * 255;
            x.at< cv::Vec3b >( i, j )[ 1 ] = c.green * 255;
            x.at< cv::Vec3b >( i, j )[ 2 ] = c.red * 255;
        }
    }
    return x;
}

af::array
ApplyColorMap::operator()( const af::array& gray, bool gpu ) // must be grayscale 
{
    // apply colorMap in CUDA kernel
    af::array rgb = afColorMap( gray, levels_, colors_ );
    return rgb;
}

template<> af::array
ApplyColorMap::operator()( const cv::Mat&, float scaleFactor, bool gpu )
{
    return af::array();
}

template<> af::array
ApplyColorMap::operator()( const af::array&, float scaleFactor, bool gpu )
{
    return af::array();
}

template<> cv::Mat
ApplyColorMap::operator()( const cv::Mat&, float scaleFactor, bool gpu )
{
    return cv::Mat();
}

template<> cv::Mat
ApplyColorMap::operator()( const af::array&, float scaleFactor, bool gpu )
{
    return cv::Mat();    
}

