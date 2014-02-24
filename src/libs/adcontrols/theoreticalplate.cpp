// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "theoreticalplate.hpp"

using namespace adcontrols;

TheoreticalPlate::TheoreticalPlate() : ntp_(0)
                                     , ntpBaselineStartTime_(0)
                                     , ntpBaselineStartHeight_(0)
                                     , ntpBaselineEndTime_(0)
                                     , ntpBaselineEndHeight_(0)
                                     , ntpPeakTopTime_(0)
                                     , ntpPeakTopHeight_(0)
{
}

TheoreticalPlate::TheoreticalPlate( const TheoreticalPlate& t )
    : ntp_( t.ntp_ )
    , ntpBaselineStartTime_( t.ntpBaselineStartTime_ )
    , ntpBaselineStartHeight_( t.ntpBaselineStartHeight_ )
    , ntpBaselineEndTime_( t.ntpBaselineEndTime_ )
    , ntpBaselineEndHeight_( t.ntpBaselineEndHeight_ )
    , ntpPeakTopTime_( t.ntpPeakTopTime_ )
    , ntpPeakTopHeight_( t.ntpPeakTopHeight_ )
{
}

double
TheoreticalPlate::ntp() const
{
    return ntp_;
}

void
TheoreticalPlate::ntp( double value )
{
    ntp_ = value;
}

double
TheoreticalPlate::baselineStartTime() const
{
    return ntpBaselineStartTime_;
}

void
TheoreticalPlate::baselineStartTime( double value )
{
    ntpBaselineStartTime_ = value;
}

double
TheoreticalPlate::baselineStartHeight() const
{
    return ntpBaselineStartHeight_;
}

void
TheoreticalPlate::baselineStartHeight( double value )
{
    ntpBaselineStartHeight_ = value;
}

double
TheoreticalPlate::baselineEndTime() const
{
    return ntpBaselineEndTime_;
}

void
TheoreticalPlate::baselineEndTime( double value )
{
    ntpBaselineEndTime_ = value;
}

double
TheoreticalPlate::baselineEndHeight() const
{
    return ntpBaselineEndHeight_;
}

void
TheoreticalPlate::baselineEndHeight( double value )
{
    ntpBaselineEndHeight_ = value;
}

double
TheoreticalPlate::peakTopTime() const
{
    return ntpPeakTopTime_;
}

void
TheoreticalPlate::peakTopTime( double value )
{
    ntpPeakTopTime_ = value;
}

double
TheoreticalPlate::peakTopHeight() const
{
    return ntpPeakTopHeight_;
}

void
TheoreticalPlate::peakTopHeight( double value )
{
    ntpPeakTopHeight_ = value;
}
