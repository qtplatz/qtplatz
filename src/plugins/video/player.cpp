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
** Reference:
** http://codingexodus.blogspot.jp/2012/12/working-with-video-using-opencv-and-qt.html
**************************************************************************/

#include "player.hpp"
#include "recorder.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcv/transform.hpp>
#include <adportable/debug.hpp>
#include <boost/filesystem.hpp>
#include <chrono>
#include <vector>

using namespace video;

Player::Player( QObject * parent ) : QThread( parent )
                                   , stop_( true )
                                   , frameRate_( 0 )
                                   , isCamera_( true )
{
}

bool
Player::loadVideo( const std::string& filename )
{
    isCamera_ = false;
    recorder_.reset();

    if ( capture_.open( filename ) ) {
        if ( capture_.isOpened() )    {
            // capture_.set( cv::CAP_PROP_CONVERT_RGB, false ); // --> request raw value
            frameRate_ = capture_.get( cv::CAP_PROP_FPS );
            if ( frameRate_ >= 1000 ) // workaround for webm that returning 1000 fps
                frameRate_ = 30;

            // save as .mp4 if not exists
            auto path = boost::filesystem::path( filename );
            if ( path.extension() != ".mp4" ) {
                path.replace_extension( ".mp4" );
                if ( ! boost::filesystem::exists( path ) ) {
                    if ( ( recorder_ = std::make_unique< Recorder >() ) ) {
                        cv::Size sz( capture_.get( cv::CAP_PROP_FRAME_WIDTH )
                                     , capture_.get( cv::CAP_PROP_FRAME_HEIGHT ) );
                        recorder_->open( path.string(), frameRate_, sz, true );
                    }
                }
            }

            return true;
        }
    }
    return false;
}

bool
Player::loadCamera( int index )
{
    recorder_.reset();
    capture_.open( index );
    if ( capture_.isOpened() )    {
        frameRate_ = capture_.get( cv::CAP_PROP_FPS );
        if ( frameRate_ > 30 )
            frameRate_ = 30;
        isCamera_ = true;

        if ( ( recorder_ = std::make_unique< Recorder >() ) ) {
            cv::Size sz( capture_.get( cv::CAP_PROP_FRAME_WIDTH )
                         , capture_.get( cv::CAP_PROP_FRAME_HEIGHT ) );

            recorder_->open( recorder_->filename() // intenral default name
                             , frameRate_
                             , sz
                             , true );
        }
        return true;
    }
    return false;
}

void
Player::Play()
{
    if (!isRunning()) {
        if (isStopped())
            stop_ = false;
        start(LowPriority);
    }
}

void
Player::run()
{
    auto start = std::chrono::high_resolution_clock::now();

    double delay = 1.0 / frameRate_;

    // std::vector< double > trace;
    auto trace = std::make_shared< adcontrols::Chromatogram >();

    while( !stop_ ) {

        double pos = capture_.get( cv::CAP_PROP_POS_MSEC ) / 1000; // ms -> s
        auto pos_frames = capture_.get( cv::CAP_PROP_POS_FRAMES );

        cv::Mat mat;
        if ( ! capture_.read( mat ) ) {
            stop_ = true;
            continue;
        }
        //----------->
        {
            std::lock_guard< std::mutex > lock( mutex_ );
            que_.emplace_back( pos_frames, pos, mat );
        }
        if ( recorder_ )
            (*recorder_) << mat;

        emit dataChanged();

        if ( isCamera_ ) {
            if ( mat.channels()== 3 ) {
                cv::cvtColor( mat, RGBframe_, cv::COLOR_BGR2RGB );
                img_ = QImage( RGBframe_.data, RGBframe_.cols, RGBframe_.rows, QImage::Format_RGB888 );
            } else {
                img_ = QImage( mat.data, mat.cols, mat.rows, QImage::Format_Indexed8 );
            }
            emit processedImage( img_ );
        }

        // no wait
        std::this_thread::sleep_until( start + std::chrono::duration< double >( pos + delay ) );
    }
}

Player::~Player()
{
    mutex.lock();
    stop_ = true;
    capture_.release();
    condition.wakeOne();
    mutex.unlock();

    wait();
}

void
Player::Stop()
{
    stop_ = true;
    //state_ = StoppedState;
}

void
Player::Next()
{
    // if ( ! threads_.empty() )
    //     __stop();
    // state_ = PausedState;
    // emit next( true );
}

void
Player::Prev()
{
}

bool
Player::isStopped() const
{
    return this->stop_;
}

void
Player::setRate( double rate )
{
}

double
Player::frameRate() const
{
    return frameRate_;
}

size_t
Player::numberOfFrames() const
{
    double value(0);
    if ( ( value = capture_.get( cv::CAP_PROP_FRAME_COUNT ) ) > 0 )
        return size_t( value );
    return 1000;
}

size_t
Player::currentFrame() const
{
    return capture_.get( cv::CAP_PROP_POS_FRAMES );
}

void
Player::setCurrentFrame( int frameNumber )
{
    capture_.set( cv::CAP_PROP_POS_FRAMES, frameNumber );
}

double
Player::currentTime() const
{
    return capture_.get( cv::CAP_PROP_POS_MSEC );
}

boost::optional< std::tuple< size_t, double, cv::Mat > >
Player::fetch()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( ! que_.empty() ) {
        auto pair = std::move( que_.front() );
        que_.pop_front();
        return pair;
    }
    return boost::none;
}

//static
QImage
Player::toImage( const cv::Mat& mat )
{
    return adcv::transform_< QImage >()( mat );
    // if ( mat.channels()== 3 ) {
    //     cv::Mat rgb;
    //     cv::cvtColor( mat, rgb, CV_BGR2RGB );
    //     return QImage( rgb.data, rgb.cols, rgb.rows, QImage::Format_RGB888 );
    // } else {
    //     return QImage( mat.data, mat.cols, mat.rows, QImage::Format_Indexed8 );
    // }
}

QImage
Player::toImage( cv::Mat&& mat )
{
    return adcv::transform_< QImage >()( mat );
}
