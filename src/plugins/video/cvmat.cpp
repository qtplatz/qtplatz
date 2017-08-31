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

#include "cvmat.hpp"
#include <adcontrols/mappedimage.hpp>
#include <opencv2/opencv.hpp>
#include <adportable/debug.hpp>
#include <boost/format.hpp>

using namespace video;

cv::Mat
cvmat::operator()( const adcontrols::MappedImage& matrix ) const
{
    cv::Mat m( matrix.size1(), matrix.size2(), CV_32FC1 );

    //double z = std::max( matrix.mergeCount(), 1UL );
    double z = matrix.max_z();

    for ( size_t i = 0; i < matrix.size1(); ++i ) {
        for ( size_t j = 0; j < matrix.size2(); ++j )
            m.at< float >( i, j ) = matrix( i, j ) / z;
    }
    return m;
}

std::shared_ptr< adcontrols::MappedImage >
cvmat::operator()( const cv::Mat& m, size_t mergeCount ) const
{
    auto dst = std::make_shared< adcontrols::MappedImage >( m.rows, m.cols );

    for ( size_t i = 0; i < m.rows; ++i ) {
        for ( size_t j = 0; j < m.cols; ++j ) {
            (*dst)( i, j ) = m.at< float >( i, j ) * mergeCount;
        }
    }
    dst->setMergeCount( mergeCount );
    return dst;
}

void
cvmat::mesh( cv::Mat & m, size_t split, size_t width ) const
{
    auto r_split = m.rows / split;
    auto c_split = m.cols / split;
    
    for ( size_t i = r_split; i < m.rows; i += r_split ) {
        for ( size_t j = 0; j < width; ++j ) {
            for ( size_t k = 0; k < m.cols; ++k ) {
                m.at< float >( i + j, k ) = float( width - j ) / width;
                m.at< float >( i - j, k ) = float( width - j ) / width;
            }
        }
    }

    for ( size_t i = c_split; i < m.cols; i += c_split ) {
        for ( size_t j = 0; j < width; ++j ) {
            for ( size_t k = 0; k < m.rows; ++k ) {
                m.at< float >( k, i + j ) = float( width - j ) / width;
                m.at< float >( k, i - j ) = float( width - j ) / width;
            }
        }
    }
    auto it = std::max_element( m.begin< float >(), m.end< float >() );

    if ( it != m.end< float >(), *it > 0 )
        m /= *it;
}

cv::Mat
cvmat::scaleLog( const cv::Mat& src )
{
    cv::Mat dst = src;
    dst += cv::Scalar::all(1);
    cv::log( dst, dst );
    return dst;
}

cvColor::cvColor( bool gray )
{
    if ( gray ) {
        colors_.emplace_back( 0.0, 0.0, 0.0, 0.00 );
        colors_.emplace_back( 1.0, 1.0, 1.0, 1.00 ); // white
    } else {
        colors_.emplace_back( 0,     0, 0.0, 0.00 );
        colors_.emplace_back( 0,     0, 0.5, 0.20 );
        colors_.emplace_back( 0,   1.0, 1.0, 0.40 ); // cyan
        colors_.emplace_back( 0,   1.0,   0, 0.60 ); // green
        colors_.emplace_back( 1.0, 1.0,   0, 0.80 ); // yellow
        colors_.emplace_back( 1.0,   0,   0, 0.97 ); // red
        colors_.emplace_back( 1.0, 1.0, 1.0, 1.00 ); // white
    }
}

QColor
cvColor::color( double value ) const
{
    auto it = std::lower_bound( colors_.begin(), colors_.end(), value, []( const auto& c, const double& v ){
            return c.value < v; } );

    if ( it == colors_.end() )
        return QColor( int(colors_.back().r * 255), int(colors_.back().g * 255), int(colors_.back().b * 255) );

    if ( it == colors_.begin() )
        return QColor( int(it->r * 255), int(it->g * 255), int(it->b * 255) );

    auto prev = it - 1;
    double frac = ( value - prev->value ) / ( it->value - prev->value );

    double r = ( it->r - prev->r ) * frac + prev->r;
    double g = ( it->g - prev->g ) * frac + prev->g;
    double b = ( it->b - prev->b ) * frac + prev->b;

    // ADDEBUG() << boost::format( "%g, frac=%g, rgb(%g, %g, %g)" ) % value % frac % r % g % b;

    return QColor( int(r*255), int(g*255), int(b*255) );
}


cv::Mat
cvColor::operator()( const adcontrols::MappedImage& matrix, int _z ) const
{
    cv::Mat m( matrix.size1(), matrix.size2(), CV_8UC3 );

    double z = _z == 0 ? std::max( matrix.mergeCount(), 1UL ) : _z;
    //double z = matrix.max_z();

    for ( size_t i = 0; i < matrix.size1(); ++i ) {
        for ( size_t j = 0; j < matrix.size2(); ++j ) {
            double v = matrix( i, j ) / z;  // [0:1] space
            auto c = color( v );

            // uint32_t d = uint32_t( v * 0x1000000 );
            m.at< cv::Vec3b >( i, j )[0] = c.blue();
            m.at< cv::Vec3b >( i, j )[1] = c.green();
            m.at< cv::Vec3b >( i, j )[2] = c.red();
        }
    }

    return m;
}

cv::Mat
cvColor::operator()( const cv::Mat& m, double scale ) const
{
    if ( m.type() != CV_32F )
        return cv::Mat();

    cv::Mat x( m.rows, m.cols, CV_8UC3 );
    for ( size_t i = 0; i < m.rows; ++i ) {
        for ( size_t j = 0; j < m.cols; ++j ) {
            double v = m.at< float >( i, j ) * scale;
            auto c = color( v );
            x.at< cv::Vec3b >( i, j )[ 0 ] = c.blue();
            x.at< cv::Vec3b >( i, j )[ 1 ] = c.green();
            x.at< cv::Vec3b >( i, j )[ 2 ] = c.red();
        }
    }

    return x;
}
