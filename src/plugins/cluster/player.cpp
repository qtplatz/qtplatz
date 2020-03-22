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
#include <adportable/debug.hpp>
#include <chrono>

using namespace cluster;

Player::Player( QObject * parent ) : QObject( parent )
                                   , trigInterval_( 10e-3 ) // 100Hz
                                   , avgCount_( 1 )
                                   , state_( StoppedState )
                                   , thread_ready_( false )
                                   , thread_signal_( false )
                                   , thread_stopping_( false )
                                   , rate_( 1.0 )
{
}

Player::~Player()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( !threads_.empty() )
        __stop();
}

void
Player::Play()
{
    state_ = PlayingState;
    tp_ = std::chrono::steady_clock::now();
    __start();
}

void
Player::Stop()
{
    if ( ! threads_.empty() )
        __stop();
    state_ = StoppedState;
}

void
Player::Next()
{
    if ( ! threads_.empty() )
        __stop();
    state_ = PausedState;
    emit next( true );
}

void
Player::Prev()
{
    if ( ! threads_.empty() )
        __stop();
    state_ = PausedState;
    emit next( false );
}

bool
Player::isStopped() const
{
    return state_ == StoppedState;
}

PlayerState
Player::state() const
{
    return state_;
}

void
Player::setTrigInterval( double value )
{
    trigInterval_ = value;
}

void
Player::setAverageCount( size_t n )
{
    avgCount_ = n;
}

void
Player::setRate( double rate )
{
    rate_ = rate;
}

void
Player::signal()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    thread_signal_ = true;
    cv_.notify_one();
}

void
Player::worker_thread()
{
    while ( ! thread_stopping_ ) {

        std::unique_lock< std::mutex > lock( mutex_ );
        cv_.wait( lock, [&]{ return thread_signal_; } );

        thread_signal_ = false;

        if ( ! thread_stopping_ ) {
#if 0
            auto nextp = tp_ + std::chrono::duration< double >( trigInterval_ * avgCount_ / rate_ );
            if ( std::chrono::duration< double >( nextp - std::chrono::steady_clock::now() ).count() < 0 )
                std::this_thread::sleep_for( std::chrono::duration< double >( trigInterval_ * avgCount_ ) );
            else
                std::this_thread::sleep_until( nextp );
#else
            std::this_thread::sleep_for( std::chrono::duration< double >( trigInterval_ * avgCount_ / rate_ ) );
#endif
            emit onSignaled();
            tp_ = std::chrono::steady_clock::now();
        }
    }
}

void
Player::__start()
{
    thread_stopping_ = false;
    {
        std::lock_guard< std::mutex > lock( mutex_ );

        if ( threads_.empty() )
            threads_.emplace_back( [&](){ worker_thread(); });
    }
    signal();
}

void
Player::__stop()
{
    if ( ! threads_.empty() ) {
        thread_stopping_ = true;
        signal();
    }

    for ( auto& t: threads_ )
        t.join();

    std::lock_guard< std::mutex > lock( mutex_ );
    threads_.clear();
}
