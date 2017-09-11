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

#include "moments.hpp"
#include "transform.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <boost/numeric/ublas/matrix.hpp>

using namespace advision;

Moments::Moments( const std::pair<int,int>& blurCount, int sizeFactor, int cannyThreshold
                  , const std::pair< unsigned, unsigned >& szThreshold )
    : blurCount_( blurCount )
    , sizeFactor_( sizeFactor )
    , cannyThreshold_( cannyThreshold )
    , szThreshold_( szThreshold )
{
}

Moments::Moments( const Moments& t )
    : blurCount_( t.blurCount_ )
    , sizeFactor_( t.sizeFactor_ )
    , cannyThreshold_( t.cannyThreshold_ )
    , szThreshold_( t.szThreshold_ )
{
}

boost::numeric::ublas::matrix< double >
Moments::operator()( const boost::numeric::ublas::matrix< double >& m ) const
{
    auto mat = transform_< cv::Mat >()( m );

    if ( sizeFactor_ > 1 )
        cv::resize( mat, mat, cv::Size(0,0), sizeFactor_, sizeFactor_, CV_INTER_LINEAR );

    mat.convertTo( mat, CV_8UC1, 255 );

    if ( blurCount_.first > 0 || blurCount_.second > 0 )
        cv::blur( mat, mat, cv::Size( blurCount_.first, blurCount_.second ) );

    cv::Canny( mat, mat, cannyThreshold_, cannyThreshold_ * 2, 3 );
        
    std::vector< std::vector< cv::Point > > contours;
    std::vector< cv::Vec4i > hierarchy;
    cv::findContours( mat, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );

    boost::numeric::ublas::matrix< double > moments( m.size1(), m.size2() );

    for( int i = 0; i< contours.size(); i++ )  {
        
        cv::Moments mu = cv::moments( contours[i], false );
        unsigned cx = unsigned ( ( mu.m10 / mu.m00 ) / sizeFactor_ );
        unsigned cy = unsigned ( ( mu.m01 / mu.m00 ) / sizeFactor_ );
        if ( cx < m.size1() && cy < m.size2() )
            moments( cx, cy ) = 1.0;
    }

    return moments;
}

void
Moments::operator()( boost::numeric::ublas::matrix< double >& out, const boost::numeric::ublas::matrix< double >& in ) const
{
    auto mat = transform_< cv::Mat >()( in );

    if ( in.size1() != out.size1() || in.size2() != out.size2() ) {
        out.resize( in.size1(), in.size2(), false );
        out.clear();
    }

    if ( sizeFactor_ > 1 )
        cv::resize( mat, mat, cv::Size(0,0), sizeFactor_, sizeFactor_, CV_INTER_LINEAR );

    mat.convertTo( mat, CV_8UC1, 255 );

    if ( blurCount_.first > 0 || blurCount_.second > 0 )
        cv::blur( mat, mat, cv::Size( blurCount_.first, blurCount_.second ) );

    cv::Canny( mat, mat, cannyThreshold_, cannyThreshold_ * 2, 3 );
        
    std::vector< std::vector< cv::Point > > contours;
    std::vector< cv::Vec4i > hierarchy;
    cv::findContours( mat, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );

    for( int i = 0; i< contours.size(); i++ )  {

        cv::Rect rc = cv::boundingRect( contours[i] );
        if ( ( rc.width >= szThreshold_.first && rc.height >= szThreshold_.first ) &&
             ( rc.width < szThreshold_.second && rc.height < szThreshold_.second ) ) {
            
            cv::Moments mu = cv::moments( contours[i], false );
            unsigned cx = unsigned( ( mu.m10 / mu.m00 ) / sizeFactor_ );
            unsigned cy = unsigned( ( mu.m01 / mu.m00 ) / sizeFactor_ );
            if ( cx < out.size1() && cy < out.size2() )
            out( cx, cy ) += 1.0;
        }
    }
}
