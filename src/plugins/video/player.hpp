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

#include <QMutex>
#include <QThread>
#include <QImage>
#include <QWaitCondition>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <deque>
#include <mutex>
#include <thread>

namespace video {

    class Recorder;

    class Player : public QThread {
        Q_OBJECT

    private:
        bool isCamera_;
        bool stop_;
        QMutex mutex;
        QWaitCondition condition;
        double frameRate_;
        cv::VideoCapture capture_;
        cv::Mat RGBframe_;
        QImage img_;
        std::mutex mutex_;

        std::deque< cv::Mat > que_;
        std::unique_ptr< Recorder > recorder_;

    signals:
        void processedImage( const QImage& image );
        void dataChanged();

    protected:
        void run();

    public:
        Player(QObject *parent = 0);
        ~Player();

        //Load a video from memory
        bool loadVideo( const std::string& filename );

        bool loadCamera( int );

        void Play();
        void Stop();
        void Next();
        void Prev();

        //check if the player has been stopped
        bool isStopped() const;

        //
        void setRate( double );
        double frameRate() const;
        size_t numberOfFrames() const;
        size_t currentFrame() const;
        double currentTime() const;
        void setCurrentFrame( int frameNumber );

        //
        bool fetch( cv::Mat& ); // remove from top
        static QImage toImage( const cv::Mat& );
    };

}
