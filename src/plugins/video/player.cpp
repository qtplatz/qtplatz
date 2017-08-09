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
    capture_.open( filename );
    
    if ( capture_.isOpened() )    {
        frameRate_ = capture_.get( CV_CAP_PROP_FPS );
        return true;
    } else
        return false;
}

bool
Player::loadCamera( int index )
{
    capture_.open( index );
    
    if ( capture_.isOpened() )    {
        frameRate_ = capture_.get( CV_CAP_PROP_FPS );
        isCamera_ = true;
        recorder_ = std::make_unique< Recorder >();
        return true;
    } else
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

    while( !stop_ ) {

        cv::Mat mat;
        
        if ( ! capture_.read( mat ) )
            stop_ = true;

        if ( mat.channels()== 3 ) {

            cv::cvtColor( mat, RGBframe_, CV_BGR2RGB );
            img_ = QImage( RGBframe_.data, RGBframe_.cols, RGBframe_.rows, QImage::Format_RGB888 );

        } else {

            img_ = QImage( mat.data, mat.cols, mat.rows, QImage::Format_Indexed8 );

        }

        emit processedImage( img_ );

        if ( isCamera_ && recorder_ )
            (*recorder_) << std::move( mat );
        
        std::this_thread::sleep_for( std::chrono::duration<double>( delay ) );

    }
}

Player::~Player()
{
    mutex_.lock();
    stop_ = true;
    capture_.release();
    condition_.wakeOne();
    mutex_.unlock();

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

