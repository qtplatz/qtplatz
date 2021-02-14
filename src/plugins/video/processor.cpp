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

#include "processor.hpp"
#include <adportable/debug.hpp>
#include <adcv/applycolormap.hpp>
#include <adcv/cvtypes.hpp>
#include <adcv/imagewidget.hpp>
#include <boost/format.hpp>
#include <opencv2/imgproc.hpp>

using namespace video;

processor::~processor()
{
}

processor::processor() : tic_( std::make_shared< adcontrols::Chromatogram >() )
                       , bp_( std::make_shared< adcontrols::Chromatogram >() )
                       , numAverage_( 0 )
{
}

void
processor::reset()
{
    tic_ = std::make_shared< adcontrols::Chromatogram >();
    bp_ = std::make_shared< adcontrols::Chromatogram >();
    avg_.reset();
}

void
processor::addFrame( size_t pos_frames, double pos, const cv::Mat& m )
{
    double min(0), max(0), sum( cv::sum( m )[ 0 ] );

    *tic_ << std::make_pair( pos, sum );

    cv::minMaxIdx( m, &min, &max );
    *bp_ << std::make_pair( pos, max );

    ADDEBUG() << boost::format("pos_frames %d, pos: %.3f, tic: %g, bp: %g") % pos_frames % (pos * 1000) % sum % max;

    frames_.emplace_back( pos_frames, pos, m );

    // average
    cv::Mat_< uchar > gray8u;
    cv::cvtColor( m, gray8u, cv::COLOR_BGR2GRAY );

    cv::Mat_< float > gray32f( m.rows, m.cols );
    gray8u.convertTo( gray32f, CV_32FC(1), 1.0/255 ); // 0..1.0 float gray scale

    ADDEBUG() << "numAverage: " << numAverage_;
    if ( !avg_ ) {
        avg_ = std::make_unique< cv::Mat_< float > >( gray32f );
        numAverage_ = 1;
    } else {
        *avg_ += gray32f;
        numAverage_++;
    }
}

std::shared_ptr< adcontrols::Chromatogram >
processor::time_profile_tic() const
{
    return tic_;
}

std::shared_ptr< adcontrols::Chromatogram >
processor::time_profile_bp() const
{
    return bp_;
}

std::pair< const cv::Mat *, size_t >
processor::avg() const
{
    return std::make_pair( avg_.get(), numAverage_ );
}
