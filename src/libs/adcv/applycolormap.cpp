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
#include "deviceinfo.hpp"
#if HAVE_CUDA
# include "colormap.hpp"
# include "device_ptr.hpp"
# if HAVE_ARRAYFIRE
#  include "afcolormap.hpp"
# endif
# if HAVE_OPENCV
#  include "cvcolormap.hpp"
# endif
#endif
#include "transform.hpp"
#include <adportable/debug.hpp>
#include <QImage>
#include <boost/numeric/ublas/matrix.hpp>
#include <algorithm>

namespace adcv {

    static const std::vector< float > __levels { 0.00f, 0.05f, 0.30f, 0.60f, 0.80f, 0.97f, 1.0f };

                                            // black, blue, cyan, green,yellow,red, white
    static const std::vector< float > __colors { 0.0, 0.0,  0.0,  0.0,  1.0,  1.0, 1.0  // R
                                               , 0.0, 0.0,  1.0,  1.0,  1.0,  0.0, 1.0  // G
                                               , 0.0, 1.0,  1.0,  0.0,  0.0,  0.0, 1.0  // B
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

    namespace cpu {

        template< typename T >
        class ColorMap_ {
            cvColor color_;
        public:
            ColorMap_( const std::vector< float >& levels
                       , const std::vector< float >& colors ) : color_( levels, colors ) {
            }

            template< typename R > T operator()( const R&, double scaleFactor ) const;
            template< typename R > T operator()( const boost::numeric::ublas::matrix< R >&, double scaleFactor ) const;
        };

        template<>
        template<>
        cv::Mat ColorMap_< cv::Mat >::operator()( const cv::Mat& gray, double scaleFactor ) const {

            cv::Mat x( gray.rows, gray.cols, CV_8UC3 );

            for ( size_t i = 0; i < gray.rows; ++i ) {
                for ( size_t j = 0; j < gray.cols; ++j ) {
                    double v = gray.at< float >( i, j ) * scaleFactor;
                    auto c = std::move( color_( v ) );
                    x.at< cv::Vec3b >( i, j )[ 0 ] = c.blue * 255; // -> BGR
                    x.at< cv::Vec3b >( i, j )[ 1 ] = c.green * 255;
                    x.at< cv::Vec3b >( i, j )[ 2 ] = c.red * 255;
                }
            }
            return x;
        }

        //--
        template<>
        template< typename R >
        QImage ColorMap_< QImage >::operator()( const boost::numeric::ublas::matrix< R >& m, double scaleFactor ) const {

            QImage x( m.size1(), m.size2(), QImage::Format_RGB888 );
            unsigned char * p = x.bits();
            for ( size_t i = 0; i < m.size1(); ++i ) {
                for ( size_t j = 0; j < m.size2(); ++j ) {
                    double v = m( i, j ) * scaleFactor;
                    auto c = std::move( color_( v ) );
                    *p++ = c.red * 255;
                    *p++ = c.green * 255;
                    *p++ = c.blue * 255;
                }
            }
            return x;
        }

        //--
        //--
        template<>
        template< typename R >
        cv::Mat ColorMap_< cv::Mat >::operator()( const boost::numeric::ublas::matrix< R >& m, double scaleFactor ) const {

            cv::Mat x( m.size1(), m.size2(), CV_8UC(3) );
            unsigned char * p = x.ptr();
            for ( size_t i = 0; i < m.size1(); ++i ) {
                for ( size_t j = 0; j < m.size2(); ++j ) {
                    double v = m( i, j ) * scaleFactor;
                    auto c = std::move( color_( v ) );
                    *p++ = c.blue * 255;  // -> BGR
                    *p++ = c.green * 255;
                    *p++ = c.red * 255;
                }
            }
            return x;
        }
        //--
        //--
        template<>
        template<>
        QImage ColorMap_< QImage >::operator()( const cv::Mat& m, double scaleFactor ) const {
            if ( m.type() != CV_32F )
                return QImage();
            QImage x( m.rows, m.cols, QImage::Format_RGB888 );
            unsigned char * p = x.bits();
            for ( size_t i = 0; i < m.rows; ++i ) {
                for ( size_t j = 0; j < m.cols; ++j ) {
                    double v = m.at<float>( i, j ) * scaleFactor;
                    auto c = std::move( color_( v ) );
                    *p++ = c.red * 255;  // <- RGB
                    *p++ = c.green * 255;
                    *p++ = c.blue * 255;
                }
            }
            return x;
        }
    }
}


using namespace adcv;

ApplyColorMap::~ApplyColorMap()
{
}

ApplyColorMap::ApplyColorMap( size_t nlevels, const float * levels, const float * colors )
    : levels_( levels, levels + nlevels )
    , colors_( colors, colors + nlevels * 3 )
{
}

ApplyColorMap::ApplyColorMap()
    : levels_( __levels )
    , colors_( __colors )
{
}

// must be grayscale
cv::Mat
ApplyColorMap::operator()( const cv::Mat& mat, float scaleFactor, cuda_algo algo ) const
{
    if ( mat.type() != CV_32F ) {
        ADDEBUG() << "ERROR: Invalid data type";
        return cv::Mat();
    }

#if HAVE_OPENCV && HAVE_CUDA
    if ( algo == cuda_direct && deviceInfo::instance()->hasCUDA() )
        return cuda::cvColorMap( levels_, colors_ )( mat * scaleFactor );
#endif

    // fallback to software color mapping
    return cpu::ColorMap_< cv::Mat >( levels_, colors_ )( mat, scaleFactor );
}

/////////////

namespace adcv {

    template<>
    template<>
    QImage
    ApplyColorMap_<QImage>::operator()<>( const boost::numeric::ublas::matrix< float >& m, float scaleFactor ) const
    {
#if HAVE_CUDA
        if ( deviceInfo::instance()->hasCUDA() )
            return adcv::transform_< QImage >()( cuda::ColorMap( levels_, colors_ )( m, scaleFactor ) );
#endif
        return cpu::ColorMap_< QImage >( levels_, colors_ )( m, scaleFactor );
    }

///////////////////////////
    template<>
    template<>
    QImage
    ApplyColorMap_<QImage>::operator()<>( const boost::numeric::ublas::matrix< double >& m, float scaleFactor ) const
    {
#if HAVE_CUDA
        if ( deviceInfo::instance()->hasCUDA() )
            return adcv::transform_< QImage >()( cuda::ColorMap( levels_, colors_ )( m, scaleFactor ) );
#endif
        return cpu::ColorMap_< QImage >( levels_, colors_ )( m, scaleFactor );
    }

///////////////////////////

#if HAVE_OPENCV
    template<>
    template<> ADCVSHARED_EXPORT cv::Mat ApplyColorMap_<cv::Mat>::operator()( const boost::numeric::ublas::matrix< double >& m, float scaleFactor ) const
    {
#if HAVE_CUDA
        if ( deviceInfo::instance()->hasCUDA() )
            return cuda::cvColorMap( levels_, colors_ )( m, scaleFactor );
#endif
        return cpu::ColorMap_< cv::Mat >( levels_, colors_ )( m, scaleFactor );
    }
#endif


#if HAVE_OPENCV
    // specialization [2]
    template<>
    template<> ADCVSHARED_EXPORT cv::Mat ApplyColorMap_<cv::Mat>::operator()( const cv::Mat& mat, float scaleFactor ) const
    {
        if ( mat.type() != CV_32F ) {
            ADDEBUG() << "ERROR: Invalid data type";
            return cv::Mat();
        }
#if HAVE_CUDA
        if ( deviceInfo::instance()->hasCUDA() )
            return cuda::cvColorMap( levels_, colors_ )( mat * scaleFactor );
#endif
        return cpu::ColorMap_< cv::Mat >( levels_, colors_ )( mat, scaleFactor );
    }
#endif

#if HAVE_ARRAYFIRE
    template<>
    template<> af::array ApplyColorMap_<af::array>::operator()( const af::array& a, float scaleFactor ) const
    {
#if HAVE_CUDA
        if ( deviceInfo::instance()->hasCUDA() )
            return cuda::afColorMap( af::array( levels_.size(), 1, levels_.data() )
                                     , af::array( 3, levels_.size(), colors_.data() ) )( a * scaleFactor );
#endif
        return af::array();
    }
#endif

#if HAVE_OPENCV
    // cv::Mat -> QImage
    template<>
    template<> QImage ApplyColorMap_<QImage>::operator()( const cv::Mat& mat, float scaleFactor ) const
    {
        if ( mat.type() != CV_32F ) {
            ADDEBUG() << "ERROR: Invalid data type";
            return QImage();
        }
#if HAVE_CUDA
        if ( deviceInfo::instance()->hasCUDA() )
            return adcv::transform_< QImage >()( cuda::ColorMap( levels_, colors_ )( mat, scaleFactor ) );
#endif
        return cpu::ColorMap_< QImage >( levels_, colors_ )( mat, scaleFactor );
    }
#endif


}
