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

#pragma once

#include "cvmat.hpp"
#include <opencv2/core/core.hpp>
#include <adcontrols/chromatogram.hpp>
#include <boost/optional.hpp>
#include <mutex>
#include <tuple>
#include <vector>

namespace adcontrols {
    class Chromatogram;
}

namespace adfs {
    class sqlite;
}

namespace video {

    class Recorder;

    class processor {
        std::mutex mutex_;
        std::vector< std::tuple< size_t, double, cv::Mat > > frames_;   // raw frames
        std::vector< std::tuple< size_t, double, cv::Mat > > cannys_;   // canny
        std::vector< std::tuple< size_t, double, cv::Mat > > contours_; // contours_ drawable
        std::shared_ptr< adcontrols::Chromatogram > tic_;     // total ion current
        std::shared_ptr< adcontrols::Chromatogram > bp_;      // base peak intensity
        std::shared_ptr< adcontrols::Chromatogram > counts_;  // contour counts
        std::unique_ptr< cv::Mat > avg_;
        size_t numAverage_;
        std::string filename_;
        std::string dbfile_;
        std::unique_ptr< adfs::sqlite > db_;
        size_t current_frame_pos_;
        std::unique_ptr< Recorder > recorder_;
    public:
        ~processor();
        processor();
        void reset();
        void addFrame( size_t pos, double t, const cv::Mat& );
        std::shared_ptr< adcontrols::Chromatogram > time_profile_tic() const;
        std::shared_ptr< adcontrols::Chromatogram > time_profile_bp() const;
        std::shared_ptr< adcontrols::Chromatogram > time_profile_counts() const;
        std::pair< const cv::Mat *, size_t > avg() const;
        boost::optional< std::tuple< size_t, double, cv::Mat > > frame( size_t frame_pos = size_t(-1) );
        boost::optional< std::tuple< size_t, double, cv::Mat > > canny( size_t frame_pos = size_t(-1) );
        boost::optional< std::tuple< size_t, double, cv::Mat > > contours( size_t frame_pos = size_t(-1) );
        void set_filename( const std::string& );
        const std::string& filename() const;
        void rewind( bool tail = false );
        size_t frame_pos_from_time( double ) const;
        void set_frame_pos( size_t );
        size_t current_frame_pos() const;
        size_t next_frame_pos( bool forward );
        Recorder * create_recorder();
        void close_recorder();
        void record( const cv::Mat& );
    };

}
