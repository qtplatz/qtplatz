/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "processor.hpp"
#include "document.hpp"
#include <adportable/debug.hpp>
#include <adcv/applycolormap.hpp>
#include <adcv/cvtypes.hpp>
#include <adcv/imagewidget.hpp>
#include <boost/format.hpp>
#include <opencv2/imgproc.hpp>

using namespace video;

processor::~processor()
{
}

processor::processor() : tic_( std::make_shared< adcontrols::Chromatogram >() )
                       , bp_( std::make_shared< adcontrols::Chromatogram >() )
                       , numAverage_( 0 )
{
}

void
processor::reset()
{
    tic_ = std::make_shared< adcontrols::Chromatogram >();
    bp_ = std::make_shared< adcontrols::Chromatogram >();
    avg_.reset();
}

void
processor::addFrame( size_t pos_frames, double pos, const cv::Mat& m )
{
    double min(0), max(0), sum( cv::sum( m )[ 0 ] );

    *tic_ << std::make_pair( pos, sum );

    cv::minMaxIdx( m, &min, &max );
    *bp_ << std::make_pair( pos, max );

    ADDEBUG() << boost::format("pos_frames:\t%d\tpos:\t%.3f,\ttic:\t%g,\tbp:\t%g") % pos_frames % (pos * 1000) % sum % max;

    frames_.emplace_back( pos_frames, pos, m );

    // average
    cv::Mat_< uchar > gray8u;
    cv::cvtColor( m, gray8u, cv::COLOR_BGR2GRAY );

    cv::Mat_< float > gray32f( m.rows, m.cols );
    gray8u.convertTo( gray32f, CV_32FC(1), 1.0/255 ); // 0..1.0 float gray scale

    if ( !avg_ ) {
        avg_ = std::make_unique< cv::Mat_< float > >( gray32f );
        numAverage_ = 1;
    } else {
        *avg_ += gray32f;
        numAverage_++;
    }

    //---------------
    cv::Mat canny;
    cv::Mat blur;
    cv::Mat gray;
    int cannyThreshold = document::instance()->cannyThreshold();
    int szFactor = 1; // std::max( 1, document::instance()->sizeFactor() );
    int blurSize = 1; // document::instance()->blurSize();

    try {
        if ( szFactor > 1 ) {
            cv::resize( m, gray, cv::Size(0,0), szFactor, szFactor, cv::INTER_LINEAR );
            gray.convertTo( gray, CV_8UC1, 255 );
        } else {
            m.convertTo( gray, CV_8UC1, 255 );
        }
    } catch ( cv::Exception& e ) {
        ADDEBUG() << e.what();
    }

    try {
        if ( blurSize >= 1 )
            cv::blur( gray, blur, cv::Size( blurSize, blurSize ) );
        else
            blur = gray;
    } catch ( cv::Exception& e ) {
        ADDEBUG() << e.what();
    }

    // edge detection
    try {
        cv::Canny( blur, canny, cannyThreshold, cannyThreshold * 2, 3 );
    } catch ( cv::Exception& e ) {
        ADDEBUG() << e.what();
    }

    cannys_.emplace_back( pos_frames, pos, canny );

    std::vector< std::vector< cv::Point > > contours;
    std::vector< cv::Vec4i > hierarchy;

    cv::findContours( canny, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );

    cv::Mat drawing = cv::Mat::zeros( canny.size(), CV_8UC3 );

    for( int i = 0; i< contours.size(); i++ )  {
        unsigned c = i + 1;
        cv::Scalar color = cv::Scalar( (c&01)*255, ((c&02)/2)*255, ((c&04)/4)*255 );
        drawContours( drawing, contours, i, color, 1, 8, hierarchy, 0, cv::Point() );

        cv::Moments mu = cv::moments( contours[i], false );
        double cx = ( mu.m10 / mu.m00 ) / szFactor;
        double cy = ( mu.m01 / mu.m00 ) / szFactor;
        double area = cv::contourArea( contours[i] ) / ( szFactor * szFactor );
        cv::Rect rc = boundingRect( contours[i] );
        size_t x = size_t( 0.5 + rc.x / szFactor );
        size_t y = size_t( 0.5 + rc.y / szFactor );
        double width = rc.width / szFactor;
        double height = rc.height / szFactor;
    } // for
    contours_.emplace_back( pos_frames, pos, drawing );
}

std::shared_ptr< adcontrols::Chromatogram >
processor::time_profile_tic() const
{
    return tic_;
}

std::shared_ptr< adcontrols::Chromatogram >
processor::time_profile_bp() const
{
    return bp_;
}

std::pair< const cv::Mat *, size_t >
processor::avg() const
{
    return std::make_pair( avg_.get(), numAverage_ );
}

boost::optional< std::tuple< size_t, double, cv::Mat > >
processor::canny( size_t frame_pos )
{
    if ( frame_pos == size_t(-1) )
        return cannys_.back();
    auto it = std::lower_bound( cannys_.begin(), cannys_.end(), frame_pos, []( const auto& a, const auto& b){
        return std::get< 0 >(a) < b;
    });
    if ( it != cannys_.end() )
        return *it;
    return boost::none;
}

boost::optional< std::tuple< size_t, double, cv::Mat > >
processor::contours( size_t frame_pos )
{
    if ( frame_pos == size_t(-1) )
        return contours_.back();
    auto it = std::lower_bound( contours_.begin(), contours_.end(), frame_pos, []( const auto& a, const auto& b){
        return std::get< 0 >(a) < b;
    });
    if ( it != contours_.end() )
        return *it;
    return boost::none;
}
