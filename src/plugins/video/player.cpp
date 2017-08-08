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

using namespace video;

Player::Player( QObject * parent ) : QThread( parent )
                                   , stop_( true )
                                   , frameRate_( 0 )
{
}

bool
Player::loadVideo( const std::string& filename )
{
    capture_.open( filename );
    
    if ( capture_.isOpened() )    {
        frameRate_ = capture_.get( CV_CAP_PROP_FPS );
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
    int delay = int(1000/frameRate_);

    while( !stop_ ){
        if ( !capture_.read(frame_) )
            stop_ = true;

        if ( frame_.channels()== 3 ) {

            cv::cvtColor(frame_, RGBframe_, CV_BGR2RGB);
            img_ = QImage(RGBframe_.data, RGBframe_.cols, RGBframe_.rows, QImage::Format_RGB888);
        } else {
            img_ = QImage((frame_.data), frame_.cols, frame_.rows, QImage::Format_Indexed8);
        }
        emit processedImage( img_ );
        this->msleep(delay);
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

void
Player::msleep(int ms)
{
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
    nanosleep(&ts, NULL);
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
    return capture_.get( CV_CAP_PROP_FRAME_COUNT );
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

