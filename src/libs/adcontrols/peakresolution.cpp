// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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

#include "peakresolution.hpp"

using namespace adcontrols;

PeakResolution::PeakResolution() : rs_(0)
                                 , rsBaselineStartTime_(0)
                                 , rsBaselineStartHeight_(0)
                                 , rsBaselineEndTime_(0)
                                 , rsBaselineEndHeight_(0)
                                 , rsPeakTopTime_(0)
                                 , rsPeakTopHeight_(0)
{
}

PeakResolution::PeakResolution( const PeakResolution& t ) : rs_( t.rs_ )
                                                          , rsBaselineStartTime_( t.rsBaselineStartTime_ )
                                                          , rsBaselineStartHeight_( t.rsBaselineStartHeight_ )
                                                          , rsBaselineEndTime_( t.rsBaselineEndTime_)
                                                          , rsBaselineEndHeight_( t.rsBaselineEndHeight_ )
                                                          , rsPeakTopTime_( t.rsPeakTopTime_ )
                                                          , rsPeakTopHeight_( t.rsPeakTopHeight_ )
{
}

double
PeakResolution::resolution() const
{
    return rs_;
}

void
PeakResolution::resolution( double rs )
{
    rs_ = rs;
}

double
PeakResolution::baselineStartTime() const
{
    return rsBaselineStartTime_;
}

void
PeakResolution::baselineStartTime( double value )
{
    rsBaselineStartTime_ = value;
}

double
PeakResolution::baselineStartHeight() const
{
    return rsBaselineStartHeight_;
}

void
PeakResolution::baselineStartHeight( double value )
{
    rsBaselineStartHeight_ = value;
}

double
PeakResolution::baselineEndTime() const
{
    return rsBaselineEndTime_;
}

void
PeakResolution::baselineEndTime( double value )
{
    rsBaselineEndTime_ = value;
}

double
PeakResolution::baselineEndHeight() const
{
    return rsBaselineEndHeight_;
}

void
PeakResolution::baselineEndHeight( double value )
{
    rsBaselineEndHeight_ = value;
}

double
PeakResolution::peakTopTime() const
{
    return rsPeakTopTime_;
}

void
PeakResolution::peakTopTime( double value )
{
    rsPeakTopTime_ = value;
}

double
PeakResolution::peakTopHeight() const
{
    return rsPeakTopHeight_;
}

void
PeakResolution::peakTopHeight( double value )
{
    rsPeakTopHeight_ = value;
}

