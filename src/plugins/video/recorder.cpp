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

#include "recorder.hpp"
#include <adcontrols/samplerun.hpp>
#include <adportable/debug.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

using namespace video;

Recorder::Recorder()
{
    auto dir = boost::filesystem::path( adcontrols::SampleRun().dataDirectory() );
    if ( ! boost::filesystem::exists( dir ) ) {
        boost::system::error_code error;
        boost::filesystem::create_directories(dir, error);        
    }
    int no = 0;
    boost::filesystem::path path;
    do {
        path = dir / ( boost::format( "video_%03d.mp4" ) % no++ ).str();
    } while ( boost::filesystem::exists( path ) );
    
    filename_ = path.string();
}

Recorder::~Recorder()
{
    writer_.release();
}

void
Recorder::operator << ( cv::Mat && mat )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    writer_.write( mat );
}

void
Recorder::operator << ( const cv::Mat& mat )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    writer_.write( mat );
}

bool
Recorder::open( const std::string& filename, double fps, cv::Size frameSize, bool isColor )
{
    filename_ = filename;
    ADDEBUG() << "##### create file: " << filename << " ######";
    
    int fourcc = CV_FOURCC('X','2','6','4');
    //int fourcc = CV_FOURCC('X','V','I','C');
    //int fourcc = CV_FOURCC('M','J','P','G');
    return writer_.open( filename, fourcc, fps, frameSize, isColor );
}
