// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "mspeakinfoitem.hpp"
#include <cstring>

using namespace adcontrols;

MSPeakInfoItem::~MSPeakInfoItem(void)
{
}

MSPeakInfoItem::MSPeakInfoItem(void)
{
    std::memset( this, 0, sizeof( MSPeakInfoItem ) );
}

unsigned int
MSPeakInfoItem::peak_index() const
{
    return peak_index_;
}

unsigned int
MSPeakInfoItem::peak_start_index() const
{
    return peak_start_index_;
}

void
MSPeakInfoItem::peak_start_index( unsigned int idx )
{
    peak_start_index_ = idx;
}

unsigned int
MSPeakInfoItem::peak_end_index() const
{
    return peak_end_index_;
}

void
MSPeakInfoItem::peak_end_index( unsigned int idx )
{
    peak_end_index_ = idx;
}

double 
MSPeakInfoItem::base_height() const
{
    return base_height_;
}

void
MSPeakInfoItem::base_height( double h )
{
    base_height_ = h;
}

double
MSPeakInfoItem::mass() const
{
    return mass_;
}

double
MSPeakInfoItem::area() const
{
    return area_;
}

double
MSPeakInfoItem::height() const
{
    return height_;
}

double
MSPeakInfoItem::widthHH( bool time ) const
{
    return time ? ( HH_right_time_ - HH_left_time_ ) : ( HH_right_mass_ - HH_left_mass_ );
}

double
MSPeakInfoItem::time( bool time ) const
{
    return time ? time_from_time_ : time_from_mass_;
}

double
MSPeakInfoItem::centroid_left( bool time ) const
{
    return time ? centroid_left_time_ : centroid_left_mass_;
}

double
MSPeakInfoItem::centroid_right( bool time ) const
{
    return time ? centroid_right_time_ : centroid_right_mass_;
}

double
MSPeakInfoItem::centroid_threshold() const
{
    return centroid_threshold_;
}

double
MSPeakInfoItem::hh_left_time() const
{
    return HH_left_time_;
}

double
MSPeakInfoItem::hh_right_time() const
{
    return HH_right_time_;
}

void
MSPeakInfoItem::assign_mass( double mass, double left, double right, double HHleft, double HHright )
{
    mass_ = mass;
    centroid_left_mass_ = left;
    centroid_right_mass_ = right;
    HH_left_mass_ = HHleft;
    HH_right_mass_ = HHright;
}
