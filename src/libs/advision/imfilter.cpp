/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

    //---------------------

    template< typename Algo >
    struct ublas_to_qimage {
        template< typename T > QImage operator()( const boost::numeric::ublas::matrix< T >& m, double scaleFactor ) const;
    };

    // matrix -> QImage gray scale RGB888
    template<>
    template< typename T > QImage
    ublas_to_qimage< imGrayScale >::operator()( const boost::numeric::ublas::matrix< T >& m, double scaleFactor ) const {

        auto minMax = find_minmax<T>()( m );

        ADDEBUG() << "ublas -> gray";
        
        QImage rgb( m.size1(), m.size2(), QImage::Format_RGB888 );
        auto p = rgb.bits();
        for ( int i = 0; i < m.size1(); ++i ) {
            for ( int j = 0; j < m.size2(); ++j ) {
                unsigned char value = m( i, j ) * 255 / minMax.second;
                *p++ = value;  // R
                *p++ = value;  // G
                *p++ = value;  // B
            }
        }
        return rgb;
    }
    
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
        return ublas_to_qimage< imGrayScale >()( m, scaleFactor );
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

}
