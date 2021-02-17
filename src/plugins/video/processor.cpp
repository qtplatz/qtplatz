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
#include "recorder.hpp"
#include <adcontrols/adcv/contoursmethod.hpp>
#include <adcv/imfilter.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
#include <adcv/minmaxidx.hpp>
#include <adcv/applycolormap.hpp>
#include <adcv/cvtypes.hpp>
#include <adcv/imagewidget.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <opencv2/imgproc.hpp>

namespace {

    struct result_writer {
        static bool open_db( adfs::sqlite& db, const std::string& file ) {
            return db.open( file.c_str(), adfs::opencreate );
        }

        static void create_table( adfs::sqlite& db ) {
            auto sql = adfs::stmt( db );
            sql.exec( "PRAGMA synchronous = OFF" );
            sql.exec( "PRAGMA journal_mode = MEMORY" );
            sql.exec( "DROP TABLE IF EXISTS frame" );
            sql.exec( "DROP TABLE IF EXISTS contours" );
            sql.exec( "CREATE TABLE IF NOT EXISTS frame ("
                      "frame_pos INTEGER"
                      ",pos REAL"
                      ",counts INTEGER"
                      ",tic INTEGER"
                      ",bp  INTEGER"
                      ");" );

            sql.exec( "CREATE TABLE IF NOT EXISTS contours ("
                      " frame_pos INTEGER"
                      ",pos REAL"
                      ",id INTEGER"
                      ",area REAL"
                      ",cx REAL"
                      ",cy REAL"
                      ",width REAL"
                      ",height REAL"
                      ",volume REAL"
                      ",cone_h REAL"
                      ",FOREIGN KEY(frame_pos) REFERENCES frame(frame_pos)"
                      ");" );
        }
        static void insert_frame( adfs::sqlite& db
                                  , size_t pos_frames, double pos, size_t sum, size_t bp, size_t counts ) {
            auto sql = adfs::stmt( db );
            sql.prepare( "INSERT INTO frame (frame_pos, pos, counts, tic, bp)"
                         " VALUES (?,?,?,?,?)" );
            sql.bind( 1 ) = pos_frames;
            sql.bind( 2 ) = pos;
            sql.bind( 3 ) = counts;
            sql.bind( 4 ) = sum;
            sql.bind( 5 ) = bp;
            if ( sql.step() != adfs::sqlite_done )
                ADDEBUG() << __FUNCTION__ << " : sqlite error.";
        }
        static void insert_contours( adfs::sqlite& db
                                     , size_t pos_frames, double pos
                                     , size_t id, double area, double cx, double cy, double width, double height
                                     , double volume, int cone_h ) {
            auto sql = adfs::stmt( db );
            sql.prepare( "INSERT INTO contours (frame_pos, pos, id, area, cx, cy, width, height,volume,cone_h)"
                         " VALUES (?,?,?,?,?,?,?,?,?,?)" );
            sql.bind( 1 ) = pos_frames;
            sql.bind( 2 ) = pos;
            sql.bind( 3 ) = id;
            sql.bind( 4 ) = area;
            sql.bind( 5 ) = cx;
            sql.bind( 6 ) = cy;
            sql.bind( 7 ) = width;
            sql.bind( 8 ) = height;
            sql.bind( 9 ) = volume;
            sql.bind( 10 ) = cone_h;
            if ( sql.step() != adfs::sqlite_done )
                ADDEBUG() << sql.errmsg();
        }
    };

    ///////////
    struct averager {
        std::unique_ptr< cv::Mat >& _;
        size_t& n_;
        averager( std::unique_ptr< cv::Mat >& a, size_t& n ) : _( a ), n_(n) {
        }
        void operator()( const cv::Mat& m ) const {
            cv::Mat_< uchar > gray8u;
            cv::cvtColor( m, gray8u, cv::COLOR_BGR2GRAY );

            cv::Mat_< float > gray32f( m.rows, m.cols );
            gray8u.convertTo( gray32f, CV_32FC(1), 1.0/255 ); // 0..1.0 float gray scale

            if ( !_ ) {
                _ = std::make_unique< cv::Mat_< float > >( gray32f );
                n_ = 1;
            } else {
                *_ += gray32f;
                n_++;
            }
        }
    };

}

using namespace video;

processor::~processor()
{
}

processor::processor() : tic_( std::make_shared< adcontrols::Chromatogram >() )
                       , bp_( std::make_shared< adcontrols::Chromatogram >() )
                       , counts_( std::make_shared< adcontrols::Chromatogram >() )
                       , numAverage_( 0 )
                       , db_( std::make_unique< adfs::sqlite >() )
                       , current_frame_pos_( 0 )
{
}

void
processor::reset()
{
    tic_ = std::make_shared< adcontrols::Chromatogram >();
    bp_ = std::make_shared< adcontrols::Chromatogram >();
    counts_ = std::make_shared< adcontrols::Chromatogram >();
    avg_.reset();
}

void
processor::addFrame( size_t pos_frames, double pos, const cv::Mat& m )
{
    double sum( cv::sum( m )[ 0 ] );

    *tic_ << std::make_pair( pos, sum );
#if __cplusplus >= 201703L
    auto [ min, max ] = adcv::minMaxIdx( m );
#else
    int min, max;
    std::tie( min, max ) = adcv::minMaxIdx( m );
#endif
    *bp_ << std::make_pair( pos, max );

    frames_.emplace_back( pos_frames, pos, m );

    // average
    averager( avg_, numAverage_ )( m );

    //---------------
    cv::Mat canny;
    cv::Mat blur;
    cv::Mat gray;
    auto cannyThreshold = document::instance()->contoursMethod().cannyThreshold();
    int szFactor = std::max( 1, document::instance()->contoursMethod().sizeFactor() );
    int blurSize = document::instance()->contoursMethod().blurSize();

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
        cv::Canny( blur, canny, cannyThreshold.first, cannyThreshold.second, 3 );
    } catch ( cv::Exception& e ) {
        ADDEBUG() << e.what();
    }

    cannys_.emplace_back( pos_frames, pos, canny );

    std::vector< std::vector< cv::Point > > contours;
    std::vector< cv::Vec4i > hierarchy;

    cv::findContours( canny, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
    *counts_ << std::make_pair( pos, contours.size() );

    result_writer::insert_frame( *db_, pos_frames, pos, sum, max, contours.size() );

    cv::Mat drawing = cv::Mat::zeros( canny.size(), CV_8UC3 );

    double volume_total(0);
    cv::Mat copy;
    m.copyTo( copy );

    for( int i = 0; i< contours.size(); i++ )  {
        unsigned c = i + 1;

        cv::Moments mu = cv::moments( contours[i], false );
        double cx = ( mu.m10 / mu.m00 ) / szFactor;
        double cy = ( mu.m01 / mu.m00 ) / szFactor;
        double area = cv::contourArea( contours[i] ) / ( szFactor * szFactor );
        cv::Rect rc = boundingRect( contours[i] );
        double width = rc.width / szFactor;
        double height = rc.height / szFactor;
        cv::Point centre( mu.m10 / mu.m00, mu.m01 / mu.m00 );

        cv::Scalar color = cv::Scalar( (c&01)*255, ((c&02)/2)*255, ((c&04)/4)*255 );
        cv::drawContours( drawing, contours, i, color, 1, 8, hierarchy, 0, cv::Point() );
        cv::drawMarker( drawing, centre, cv::Scalar( 255, 0, 0 ), cv::MARKER_CROSS, std::min( rc.width, rc.height), 1, 8 );

        cv::Mat roi( m, rc );
        double volume = cv::sum( roi )[0];
        volume_total += volume;
#if __cplusplus >= 201703L
        auto [ min, cone_h ] = adcv::minMaxIdx( roi );
#else
        int min, cone_h;
        std::tie( min, cone_h ) = adcv::minMaxIdx( roi );
#endif
        result_writer::insert_contours( *db_, pos_frames, pos, i, area, cx, cy, width, height, volume, cone_h );
        cv::Mat zroi( copy, rc );
        zroi.setTo( cv::Scalar( 0, 0, 0 ) );
    } // for

    auto dark = cv::sum( copy )[0];
    ADDEBUG() << boost::format("pos_frames:\t%4d\tpos:\t%7.2f,\ttic:\t%g\tdark:\t%g\tbp:\t%d\tn:\t%d")
        % pos_frames % (pos * 1000) % sum % dark % max % contours.size();

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

std::shared_ptr< adcontrols::Chromatogram >
processor::time_profile_counts() const
{
    return counts_;
}

std::pair< const cv::Mat *, size_t >
processor::avg() const
{
    return std::make_pair( avg_.get(), numAverage_ );
}

boost::optional< std::tuple< size_t, double, cv::Mat > >
processor::frame( size_t frame_pos )
{
    if ( frame_pos == size_t(-1) )
        return frames_.back();
    auto it = std::lower_bound( frames_.begin(), frames_.end(), frame_pos, []( const auto& a, const auto& b){
        return std::get< 0 >(a) < b;
    });
    if ( it != frames_.end() )
        return *it;
    return boost::none;
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

void
processor::set_filename( const std::string& name )
{
    filename_ = name;
    boost::filesystem::path path( name );
    dbfile_ = path.replace_extension( ".db" ).string();
    if ( result_writer::open_db( *db_, dbfile_ ) )
        result_writer::create_table( *db_ );
}

const std::string&
processor::filename() const
{
    return filename_;
}

void
processor::rewind( bool tail )
{
    if ( tail )
        current_frame_pos_ = frames_.size();
    else
        current_frame_pos_ = 0;
}

size_t
processor::current_frame_pos() const
{
    return current_frame_pos_;
}

size_t
processor::next_frame_pos( bool forward )
{
    if ( forward ) {
        if ( frames_.size() > current_frame_pos_ )
            ++current_frame_pos_;
    } else {
        if ( current_frame_pos_ )
            --current_frame_pos_;
    }
    return current_frame_pos_;
}

Recorder *
processor::create_recorder()
{
    recorder_ = std::make_unique< Recorder >();
    return recorder_.get();
}

void
processor::close_recorder()
{
    recorder_.reset();
}

void
processor::record( const cv::Mat& m )
{
    if ( recorder_ )
        (*recorder_) << m;
}
