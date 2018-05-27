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
** Reference:
** http://codingexodus.blogspot.jp/2012/12/working-with-video-using-opencv-and-qt.html
**************************************************************************/

#include "player.hpp"
#include "recorder.hpp"
#include <advision/transform.hpp>
#include <boost/filesystem.hpp>
#include <chrono>

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
            frameRate_ = capture_.get( CV_CAP_PROP_FPS );
            if ( frameRate_ > 30 )
                frameRate_ = 30; // workaround for webm that returning 1000 fps

            // save as .mp4 if not exists
            auto path = boost::filesystem::path( filename );
            if ( path.extension() != ".mp4" ) {
                path.replace_extension( ".mp4" );
                if ( ! boost::filesystem::exists( path ) ) {
                    if ( ( recorder_ = std::make_unique< Recorder >() ) ) {
                        cv::Size sz( capture_.get( CV_CAP_PROP_FRAME_WIDTH )
                                     , capture_.get( CV_CAP_PROP_FRAME_HEIGHT ) );
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
        frameRate_ = capture_.get( CV_CAP_PROP_FPS );
        if ( frameRate_ > 30 )
            frameRate_ = 30;
        isCamera_ = true;
        if ( ( recorder_ = std::make_unique< Recorder >() ) ) {

            cv::Size sz( capture_.get( CV_CAP_PROP_FRAME_WIDTH )
                         , capture_.get( CV_CAP_PROP_FRAME_HEIGHT ) );
            
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
    size_t frame_counts = 0;
    auto start = std::chrono::high_resolution_clock::now();
    
    double delay = 1.0 / frameRate_;

    while( !stop_ ) {
        {
            cv::Mat mat;
            if ( ! capture_.read( mat ) )
                stop_ = true;
            std::lock_guard< std::mutex > lock( mutex_ );
            que_.emplace_back( std::move( mat ) );
        }
        emit dataChanged();

        const cv::Mat& mat = que_.back();
        if ( recorder_ )
            (*recorder_) << que_.back();
            
        if ( isCamera_ ) {
            if ( mat.channels()== 3 ) {
                cv::cvtColor( mat, RGBframe_, CV_BGR2RGB );
                img_ = QImage( RGBframe_.data, RGBframe_.cols, RGBframe_.rows, QImage::Format_RGB888 );
            } else {
                img_ = QImage( mat.data, mat.cols, mat.rows, QImage::Format_Indexed8 );
                }
            emit processedImage( img_ );                
        }

        // no wait
        std::this_thread::sleep_until( start + ( ++frame_counts * std::chrono::duration< double >( delay ) ) );
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
}

bool
Player::isStopped() const
{
    return this->stop_;
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
    if ( ( value = capture_.get( CV_CAP_PROP_FRAME_COUNT ) ) > 0 )
        return size_t( value );
    return 1000;
}

size_t
Player::currentFrame() const
{
    return capture_.get( CV_CAP_PROP_POS_FRAMES );
}

void
Player::setCurrentFrame( int frameNumber )
{
    capture_.set( CV_CAP_PROP_POS_FRAMES, frameNumber );
}

double
Player::currentTime() const
{
    return capture_.get( CV_CAP_PROP_POS_MSEC );
}

bool
Player::fetch( cv::Mat& mat )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    
    if ( !que_.empty() ) {
        mat = std::move( que_.back() );
        que_.pop_front();
        return ! mat.empty();
    }
    return false;
}

//static
QImage
Player::toImage( const cv::Mat& mat )
{
    return advision::transform_< QImage >()( mat );
    // if ( mat.channels()== 3 ) {
    //     cv::Mat rgb;
    //     cv::cvtColor( mat, rgb, CV_BGR2RGB );
    //     return QImage( rgb.data, rgb.cols, rgb.rows, QImage::Format_RGB888 );
    // } else {
    //     return QImage( mat.data, mat.cols, mat.rows, QImage::Format_Indexed8 );
    // }
}


