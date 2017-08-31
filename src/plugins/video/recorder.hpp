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

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <condition_variable>
#include <deque>
#include <thread>

namespace video {

    class Recorder {
    public:
        Recorder();
        ~Recorder();

        const std::string& filename() const { return filename_; };

        bool open( const std::string&, double fps, cv::Size frameSize, bool isColro );

        void operator << ( cv::Mat && );
        void operator << ( const cv::Mat& );

    private:
        std::mutex mutex_;
        std::condition_variable cv_;
        std::string filename_;
        cv::VideoWriter writer_;
    };

}

