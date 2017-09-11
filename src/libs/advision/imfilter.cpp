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
        cv::Size Size( const imBlur& blur ) { return cv::Size( blur.size.first, blur.size.second ); }
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

        if ( mat.rows < 256 )
            cv::resize( mat, mat, cv::Size(0,0), 256/mat.cols, 256/mat.rows, CV_INTER_LINEAR );

        cv::Size sz = size_ == 1 ? opencv::Size( std::get<0>( algos_ ) ) : cv::Size( 5, 5 );
        
        cv::GaussianBlur( mat, mat, cv::Size( 5, 5 ), 0, 0 );

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
        
        if ( mat.rows < 256 )
            cv::resize( mat, mat, cv::Size(0,0), 256/mat.cols, 256/mat.rows, CV_INTER_LINEAR );

        cv::Size sz = size_ == 2 ? opencv::Size( std::get<1>( algos_ ) ) : cv::Size( 5, 5 );
        
        cv::GaussianBlur( mat, mat, sz, 0, 0 );

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
        
        if ( mat.rows < 256 )
            cv::resize( mat, mat, cv::Size(0,0), 256/mat.cols, 256/mat.rows, CV_INTER_LINEAR );
        
        cv::Size sz = size_ == 2 ? opencv::Size( std::get<1>( algos_ ) ) : cv::Size( 5, 5 );

        cv::GaussianBlur( mat, mat, sz, 0, 0 );

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

        // --> Blur cv::Mat
        if ( mat.rows < 256 )
            cv::resize( mat, mat, cv::Size(0,0), 256/mat.cols, 256/mat.rows, CV_INTER_LINEAR );
        cv::Size sz = size_ == 2 ? opencv::Size( std::get<2>( algos_ ) ) : cv::Size( 5, 5 );
        cv::GaussianBlur( mat, mat, sz, 0, 0 );

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

        // --> Blur cv::Mat
        if ( mat.rows < 256 )
            cv::resize( mat, mat, cv::Size(0,0), 256/mat.cols, 256/mat.rows, CV_INTER_LINEAR );
        cv::Size sz = size_ == 2 ? opencv::Size( std::get<2>( algos_ ) ) : cv::Size( 5, 5 );
        cv::GaussianBlur( mat, mat, sz, 0, 0 );

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

        mat.convertTo( mat, CV_8UC1, 255 );

        if ( method.blurSize() > 0 )
            cv::blur( mat, mat, cv::Size( method.blurSize(), method.blurSize() ) );

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

            if ( ( rc.width >= method.minSizeThreshold() && rc.height >= method.minSizeThreshold() ) &&
                 ( rc.width < method.maxSizeThreshold() && rc.height < method.maxSizeThreshold() ) ) {            
                
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
    
}
