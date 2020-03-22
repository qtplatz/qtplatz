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
** Referencies;
** http://codingexodus.blogspot.jp/2012/12/working-with-video-using-opencv-and-qt.html
** http://codingexodus.blogspot.co.uk/2013/05/working-with-video-using-opencv-and-qt.html
**************************************************************************/

#pragma once

#include <QObject>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include <deque>
#include <mutex>
#include <thread>


namespace cluster {

    enum PlayerState { StoppedState, PlayingState, PausedState };

    class Recorder;

    class Player : public QObject {
        Q_OBJECT

    signals:
        void next( bool forward );
        void onSignaled();

    protected:
        void run();

    public:
        Player(QObject *parent = 0);
        ~Player();

        void Play();
        void Stop();
        void Next();
        void Prev();

        void signal();

        PlayerState state() const;

        bool isStopped() const;

        double frameRate() const;
        void setTrigInterval( double );
        void setAverageCount( size_t );
        void setRate( double );

    private:
        void __start();
        void __stop();
        void worker_thread();
        std::vector< std::thread > threads_;

        std::mutex mutex_;
        std::condition_variable cv_;
        std::atomic_bool thread_ready_;
        bool thread_signal_;
        std::atomic_bool thread_stopping_;

        double trigInterval_;
        size_t avgCount_;
        PlayerState state_;
        double rate_;
        std::chrono::steady_clock::time_point tp_;
    };

}
