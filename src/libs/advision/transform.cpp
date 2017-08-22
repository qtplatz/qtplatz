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

#include "transform.hpp"
#include "cvtypes.hpp"
#include "aftypes.hpp"
#include <arrayfire.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>

using namespace advision;

cv::Mat
transform::mat( const af::array& a )
{
    unsigned ndims = a.numdims();
    unsigned channels( 1 ), cols( 1 ), rows( 1 );
    if ( ndims == 3 ) {
        cols = a.dims( 0 );
        rows = a.dims( 1 );
        channels = a.dims( 2 );
    }

    af::array cv_ordered = af::reorder( a.T(), 2, 0, 1 );
    cv::Mat m;
    switch( a.type() ) {
    case f32:
        m = cv::Mat( rows, cols, CV_32FC( channels ) );
        cv_ordered.as( f32 ).host( m.ptr< float >() );
        break;
    case c32:
        //m = cv::Mat( rows, cols, CV_32FC( channels ) );
        //cv_ordered.as( f32 ).host( m.ptr< float >() );
        break;
    case f64:
        m = cv::Mat( rows, cols, CV_64FC( channels ) );
        cv_ordered.as( f32 ).host( m.ptr< float >() );
        break;
    case c64:
        //m = cv::Mat( rows, cols, CV_64FC( channels ) );
        //cv_ordered.as( f32 ).host( m.ptr< float >() );
        break;
    case b8:
        m = cv::Mat( rows, cols, CV_8SC( channels ) );
        cv_ordered.host( m.ptr< int8_t >() );
        break;
    case s32:
        m = cv::Mat( rows, cols, CV_32SC( channels ) );
        cv_ordered.host( m.ptr< int32_t >() );
        break;
    case u32:
        m = cv::Mat( rows, cols, CV_32SC( channels ) ); // cv::Mat has no 32UC
        cv_ordered.as( s32 ).host( m.ptr< uint32_t >() );
        break;
    case u8:
        m = cv::Mat( rows, cols, CV_8UC( channels ) );
        cv_ordered.host( m.ptr< uint8_t >() );
        break;
    case s64:
        m = cv::Mat( rows, cols, CV_64FC( channels ) ); // cv::Mat has no int64_t
        cv_ordered.as( f64 ).host( m.ptr< double >() );
        break;
    case u64:
        m = cv::Mat( rows, cols, CV_64FC( channels ) );
        cv_ordered.as( f64 ).host( m.ptr< double >() );
        break;
    case s16:
        m = cv::Mat( rows, cols, CV_16SC( channels ) );
        cv_ordered.host( m.ptr< int16_t >() );
        break;
    case u16:
        m = cv::Mat( rows, cols, CV_16UC( channels ) );
        cv_ordered.host( m.ptr< uint16_t >() );
        break;
    }
    return m;
}

af::array
transform::array( const cv::Mat& m )
{
    uint32_t ndims = ( ( unsigned( m.type() ) >> 3 ) & 07 ) + 1;
    uint32_t ntype = m.type() & 07;

    if ( ndims > 0 && ndims <= 3 ) {  // Gray .. RGB
        af::array a;
        switch( ntype ) {
        case CV_8U:
            a = af::array( ndims, m.cols, m.rows, m.ptr< cv_type< CV_8U >::type >() );
            break;
        case CV_8S:
            // signed char on af::array is boolean array
            a = af::array( ndims, m.cols, m.rows, m.ptr< unsigned char >() );
            break;
        case CV_16U:
            a = af::array( ndims, m.cols, m.rows, m.ptr< cv_type< CV_16U >::type >() );
            break;
        case CV_16S:
            a = af::array( ndims, m.cols, m.rows, m.ptr< cv_type< CV_16S >::type >() );
            break;
        case CV_32S:
            a = af::array( ndims, m.cols, m.rows, m.ptr< cv_type< CV_32S >::type >() );
            break;
        case CV_32F:
            a = af::array( ndims, m.cols, m.rows, m.ptr< cv_type< CV_32F >::type >() );
            break;
        case CV_64F:
            a = af::array( ndims, m.cols, m.rows, m.ptr< cv_type< CV_64F >::type >() );
            break;
        }
        return af::reorder( a, 1, 2, 0 ).T();
    }
    return af::array();
}

namespace advision {

    template<>
    template<> af::array transform_< af::array >::operator()< cv::Mat >( const cv::Mat& m ) const {
        return transform::array( m );
    }

    template<>
    template<> cv::Mat transform_< cv::Mat >::operator()< af::array >( const af::array& a ) const {
        return transform::mat( a );
    }

    template<>
    af::array transform_< af::array >::bgr2rgb( const af::array& a ) const {
        af::array b( a );
        b( af::span, af::span, 2 ) = a( af::span, af::span, 0 );
        b( af::span, af::span, 0 ) = a( af::span, af::span, 2 );
        return b;
    }

    template<>
    cv::Mat transform_< cv::Mat >::bgr2rgb( const cv::Mat& a ) const {
        cv::Mat b;
        cv::cvtColor( a, b, CV_RGB2BGR );
        return b;
    }
    
}

