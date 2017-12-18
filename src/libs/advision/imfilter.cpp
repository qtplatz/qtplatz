/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
**************************************************************************/

#include "imfilter.hpp"
#include "transform.hpp"
#include "applycolormap.hpp"
#include "dft2d.hpp"
#include <adportable/debug.hpp>
#include <QImage>
#if HAVE_OPENCV
# include <opencv2/core/core.hpp>
# include <opencv2/opencv.hpp>
# include <opencv2/imgproc/imgproc.hpp>
#endif
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/format.hpp>

namespace advision {

    template< typename T > struct find_minmax {
        inline std::pair< T, T > operator()( const boost::numeric::ublas::matrix< T >& m ) const {
            std::pair<T, T> mm = { m(0, 0), m(0, 0) };
            for ( int i = 0; i < m.size1(); ++i ) {
                for ( int j = 0; j < m.size2(); ++j ) {
                    mm.first = std::min( mm.first, m( i, j ) );
                    mm.second = std::max( mm.second, m( i, j ) );
                }
            }
            return mm;
        }
    };

    namespace opencv {
        inline cv::Size ksize( const imBlur& blur ) { return cv::Size( blur.ksize.first, blur.ksize.second ); }
        inline cv::Size anchor( const imBlur& blur ) { return cv::Point( blur.anchor.first, blur.anchor.second ); }

        inline bool applyGaussianBlur( const cv::Mat& in, cv::Mat& out, const imBlur& blur ) {
            cv::Size ksize = cv::Size( blur.ksize.first | 1, blur.ksize.second | 1 ); // make odd values
            try {
                cv::GaussianBlur( in, out, ksize, 0, 0 );
                return true;
            } catch ( cv::Exception& e ) {
                ADDEBUG() << "cv::Exception: " << e.what();
                return false;
            }
        }
        
        inline bool applyBlur( const cv::Mat& in, cv::Mat& out, const imBlur& blur ) {
            cv::Size ksize = cv::Size( blur.ksize.first, blur.ksize.second );
            try {
                cv::blur( in, out, opencv::ksize(blur), opencv::anchor(blur), cv::BORDER_DEFAULT );
                return true;
            } catch ( cv::Exception& e ) {
                ADDEBUG() << "cv::Exception: " << e.what();
                return false;
            }
        }
    };
    
}

class QPaintEvent;

//using namespace advision;

namespace advision {

///// GrayScale
    template<>
    template<>
    QImage
    imfilter< QImage, imGrayScale >::operator()<>( const boost::numeric::ublas::matrix< double >& m, double scaleFactor ) const
    {
        const std::vector< float > __levels{ 0.0, 1.0 };
        const std::vector< float > __colors{ 0.0, 1.0,   0.0, 1.0,   0.0, 1.0 };

        return ApplyColorMap_< QImage >(2, __levels.data(), __colors.data() )( m, float( scaleFactor ) );
    }

////// Blur
    template<>
    template<>
    QImage
    imfilter< QImage, imBlur >::operator()<>( const boost::numeric::ublas::matrix< double >& m, double scaleFactor ) const
    {
        cv::Mat mat = ApplyColorMap_< cv::Mat >()( m, float( scaleFactor ) );

        imBlur blur = size_ == 1 ? std::get<0>( algos_ ) : imBlur();
        
        if ( blur.resizeFactor > 1 )
            cv::resize( mat, mat, cv::Size(0,0), blur.resizeFactor, blur.resizeFactor, CV_INTER_LINEAR );

        opencv::applyGaussianBlur( mat, mat, blur );
        //opencv::applyBlur( mat, mat, blur );

        return transform_< QImage >()( mat );
    }

////// ColorMap matrix<double>
    template<>
    template<>
    QImage
    imfilter< QImage, imColorMap >::operator()<>( const boost::numeric::ublas::matrix< double >& m, double scaleFactor ) const
    {
        return ApplyColorMap_<QImage>()( m, scaleFactor );
    }

    //////////////////
    //////////////////
    //////////////////
    // GrayScale + Blur
    template<>
    template<>
    QImage
    imfilter< QImage
              , imGrayScale
              , imBlur >::operator()<>( const boost::numeric::ublas::matrix< double >& m, double scaleFactor ) const
    {
        const std::vector< float > __levels{ 0.0, 1.0 };
        const std::vector< float > __colors{ 0.0, 1.0,   0.0, 1.0,   0.0, 1.0 };

        cv::Mat mat = ApplyColorMap_< cv::Mat >( 2, __levels.data(), __colors.data() )( m, float( scaleFactor ) );

        imBlur blur = ( size_ == 2 ) ? std::get<1>( algos_ ) : imBlur();

        if ( blur.resizeFactor > 1 )
            cv::resize( mat, mat, cv::Size(0,0), blur.resizeFactor, blur.resizeFactor, CV_INTER_LINEAR );

        opencv::applyGaussianBlur( mat, mat, blur );
        //opencv::applyBlur( mat, mat, blur );

        return transform_< QImage >()( mat );
    }

    // ColorMap + Blur
    template<>
    template<>
    QImage
    imfilter< QImage
              , imColorMap
              , imBlur >::operator()<>( const boost::numeric::ublas::matrix< double >& m, double scaleFactor ) const
    {
        cv::Mat mat = ApplyColorMap_< cv::Mat >()( m, float( scaleFactor ) );

        imBlur blur = ( size_ == 2 ) ? std::get< 1 >( algos_ ) : imBlur();

        if ( blur.resizeFactor > 1 )
            cv::resize( mat, mat, cv::Size(0,0), blur.resizeFactor, blur.resizeFactor, CV_INTER_LINEAR );

        opencv::applyGaussianBlur( mat, mat, blur );
        //opencv::applyBlur( mat, mat, blur );
#if 0
        static int count = 0;
        std::string file = ( boost::format("/home/toshi/Pictures/debug%1%.jpg") % ++count ).str();
        ADDEBUG() << "imwrite(" << file << ")";
        cv::imwrite( file, mat );
        auto qImage = transform_< QImage >()( mat );
        qImage.save( QString("/home/toshi/Pictures/qimage%1.jpg").arg(count) );
        return qImage;
#endif
        return transform_< QImage >()( mat );
    }

    //////// ColorMap + DFT
    template<>
    template<>
    QImage
    imfilter< QImage
              , imColorMap
              , imDFT >::operator()<>( const boost::numeric::ublas::matrix< double >& m, double scaleFactor ) const
    {
        auto mat = transform_< cv::Mat >()( m ); // -> float *
        mat = dft2d().dft( mat );

        return ApplyColorMap_< QImage >()( mat, float( scaleFactor ) );
    }

    // GrayScale + DFT
    template<>
    template<>
    QImage
    imfilter< QImage
              , imGrayScale
              , imDFT >::operator()<>( const boost::numeric::ublas::matrix< double >& m, double scaleFactor ) const
    {
        auto mat = transform_< cv::Mat >()( m ); // -> float *

        mat = dft2d().dft( mat );
        
        const std::vector< float > __levels{ 0.0, 1.0 };
        const std::vector< float > __colors{ 0.0, 1.0,   0.0, 1.0,   0.0, 1.0 };

        return ApplyColorMap_< QImage >( 2, __levels.data(), __colors.data() )( mat, float( scaleFactor ) );
    }


    // ColorMap + DFT + Blur
    template<>
    template<>
    QImage
    imfilter< QImage
              , imColorMap
              , imDFT
              , imBlur >::operator()<>( const boost::numeric::ublas::matrix< double >& m, double scaleFactor ) const
    {
        // --> cv::Mat_<float>
        auto mat = transform_< cv::Mat >()( m ); // -> float *

        // --> DFT
        mat = dft2d().dft( mat );

        // --> ColorMap cv::Mat
        mat = ApplyColorMap_< cv::Mat >()( mat, float( scaleFactor ) );

        imBlur blur = ( size_ == 3 ) ? std::get<2>( algos_ ) : imBlur();

        if ( blur.resizeFactor > 1 )
            cv::resize( mat, mat, cv::Size(0,0), blur.resizeFactor, blur.resizeFactor, CV_INTER_LINEAR );        

        opencv::applyGaussianBlur( mat, mat, blur );
        //opencv::applyBlur( mat, mat, blur );

        // wrap up
        return transform_< QImage >()( mat );
    }

    // GrayScale + DFT + Blur
    template<>
    template<>
    QImage
    imfilter< QImage
              , imGrayScale
              , imDFT
              , imBlur >::operator()<>( const boost::numeric::ublas::matrix< double >& m, double scaleFactor ) const
    {
        // --> cv::Mat_<float>
        auto mat = transform_< cv::Mat >()( m ); // -> float *

        // --> DFT
        mat = dft2d().dft( mat );

        // --> GrayScale cv::Mat
        const std::vector< float > __levels{ 0.0, 1.0 };
        const std::vector< float > __colors{ 0.0, 1.0,   0.0, 1.0,   0.0, 1.0 };
        mat = ApplyColorMap_< cv::Mat >( 2, __levels.data(), __colors.data() )( mat, float( scaleFactor ) );

        imBlur blur = size_ == 3 ? std::get<2>( algos_ ) : imBlur();
        
        if ( blur.resizeFactor > 1 )
            cv::resize( mat, mat, cv::Size(0,0), blur.resizeFactor, blur.resizeFactor, CV_INTER_LINEAR );

        opencv::applyGaussianBlur( mat, mat, blur );
        //opencv::applyBlur( mat, mat, blur );

        // wrap up
        return transform_< QImage >()( mat );
    }

    //////////// Contours
    template<>
    template<>
    QImage imfilter< QImage, imContours >::operator()<>( const boost::numeric::ublas::matrix< double >& m, double scaleFactor ) const
    {
        // --> cv::Mat_<float>
        auto mat = transform_< cv::Mat >()( m );

        imContours method;
        if ( size_ >= 1 )
            method = std::get< 0 >( algos_ );

        if ( method.sizeFactor() > 1 )
            cv::resize( mat, mat, cv::Size(0,0), method.sizeFactor(), method.sizeFactor(), CV_INTER_LINEAR );

        mat.convertTo( mat, CV_8UC1, 255 * scaleFactor );

        if ( method.blurSize() > 0 ) {
            
            opencv::applyGaussianBlur( mat, mat, imBlur( { method.blurSize(), method.blurSize() } ) );
            //opencv::applyBlur( mat, mat, imBlur( { method.blurSize(), method.blurSize() } ) );
        }

        cv::Canny( mat, mat, method.cannyThreshold(), method.cannyThreshold() * 2, 3 );
        
        std::vector< std::vector< cv::Point > > contours;
        std::vector< cv::Vec4i > hierarchy;
        cv::findContours( mat, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );

        cv::Mat drawing = cv::Mat::zeros( mat.size(), CV_8UC3 );
        int c = 1;
        for( int i = 0; i< contours.size(); i++ )  {
            cv::Rect rc = boundingRect( contours[i] );
            cv::Moments mu = cv::moments( contours[i], false );
                
            double cx = ( mu.m10 / mu.m00 ) / method.sizeFactor();
            double cy = ( mu.m01 / mu.m00 ) / method.sizeFactor();
            cv::Point centre( mu.m10 / mu.m00, mu.m01 / mu.m00 );
            double area = cv::contourArea( contours[i] ) / ( method.sizeFactor() * method.sizeFactor() );

            if ( ( unsigned( rc.width ) >= method.minSizeThreshold() && unsigned( rc.height ) >= method.minSizeThreshold() ) &&
                 ( unsigned( rc.width ) < method.maxSizeThreshold() && unsigned( rc.height ) < method.maxSizeThreshold() ) ) {            
                
                cv::Scalar color = cv::Scalar( (c&01)*255, ((c&02)/2)*255, ((c&04)/4)*255 );
                cv::drawContours( drawing, contours, i, color, 1, 8, hierarchy, 0, cv::Point() );

                color = cv::Scalar( (c&01)*127, ((c&02)/2)*127, ((c&04)/4)*127 );                
                cv::drawMarker( drawing, centre, color, cv::MARKER_CROSS, std::min( rc.width, rc.height), 1, 8 );

            } else {

                cv::Scalar color = cv::Scalar( (c&01)*127, ((c&02)/2)*127, ((c&04)/4)*127 );
                cv::drawContours( drawing, contours, i, color, 1, 8, hierarchy, 0, cv::Point() );

                color = cv::Scalar( (c&01)*63, ((c&02)/2)*63, ((c&04)/4)*63 );                
                cv::drawMarker( drawing, centre, color, cv::MARKER_CROSS, std::min( rc.width, rc.height), 1, 8 );

            }
            ++c;
        }
        return advision::transform_< QImage >()( drawing );
    }
    
#ifdef WIN32
    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imGrayScale >::imfilter() : size_( 1 ) {}
    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imGrayScale >::~imfilter() {}

    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imColorMap >::imfilter() : size_( 1 ) {}
    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imColorMap >::~imfilter() {}

    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imGrayScale, imBlur >::imfilter() : size_( 2 ) {}
    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imGrayScale, imBlur >::imfilter( imGrayScale a, imBlur b ) : size_( 2 ), algos_( a, b ) {}
    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imGrayScale, imBlur >::~imfilter() {}

    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imGrayScale, imDFT >::imfilter() : size_( 2 ) {}
    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imGrayScale, imDFT >::imfilter( imGrayScale a, imDFT b ) : size_( 2 ), algos_( a, b ) {}
    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imGrayScale, imDFT >::~imfilter() {}
    
    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imColorMap, imBlur >::imfilter() : size_( 2 ) {}
    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imColorMap, imBlur >::imfilter( imColorMap a, imBlur b ) : size_( 2 ), algos_( a, b ) {}
    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imColorMap, imBlur >::~imfilter() {}

    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imColorMap, imDFT >::imfilter() : size_( 2 ) {}
    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imColorMap, imDFT >::imfilter( imColorMap a, imDFT b ) : size_( 2 ), algos_( a, b ) {}
    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imColorMap, imDFT >::~imfilter() {}

    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imColorMap, imDFT, imBlur >::imfilter() : size_( 3 ) {}
    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imColorMap, imDFT, imBlur >::imfilter( imColorMap a, imDFT b, imBlur c )
        : size_( 3 ), algos_( a, b, c ) {}
    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imColorMap, imDFT, imBlur >::~imfilter() {}

    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imGrayScale, imDFT, imBlur >::imfilter() : size_( 3 ) {}
    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imGrayScale, imDFT, imBlur >::imfilter( imGrayScale a, imDFT b, imBlur c )
        : size_( 3 ), algos_( a, b, c ) {}
    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imGrayScale, imDFT, imBlur >::~imfilter() {}
    
    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imContours >::imfilter() : size_( 1 ) {}
    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imContours >::imfilter( imContours a ) : size_( 1 ), algos_( a ) {}
    template<> ADVISIONSHARED_EXPORT imfilter< QImage, imContours >::~imfilter() {}
    
#endif

}
