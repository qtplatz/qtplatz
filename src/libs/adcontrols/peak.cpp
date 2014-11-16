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

#include "peak.hpp"
using namespace adcontrols;

Peak::~Peak()
{
}

Peak::Peak( ) : parentId_( 0 )
              , peakid_( 0 )
              , baseid_( 0 )
              , appliedFunctions_( 0 )
              , peak_flags_( 0 )
              , startPos_( 0 )
              , topPos_( 0 )
              , endPos_( 0 )
              , startTime_( 0 )
              , peakTime_( 0 )
              , endTime_( 0 )
              , startHeight_( 0 )
              , topHeight_( 0 )
              , endHeight_( 0 )
              , peakArea_( 0 )
              , peakHeight_( 0 )
              , capacityFactor_( 0 )
              , peakWidth_( 0 )
              , peakAmount_( 0 )
              , migrationTime_( 0 )
              , peakEfficiency_( 0 )
              , massOnColumn_( 0 )
              , percentArea_( 0 )
              , percentHeight_( 0 )
              , manuallyModified_( 0 )
{
}

Peak::Peak( const Peak& t ) : name_( t.name_ )
                            , parentId_( t.parentId_ )
                            , peakid_( t.peakid_ )
                            , baseid_( t.baseid_ )
                            , appliedFunctions_( t.appliedFunctions_ )
                            , peak_flags_( t.peak_flags_ )
                            , startPos_( t.startPos_ )
                            , topPos_( t.topPos_ )
                            , endPos_( t.endPos_ )
                            , startTime_( t.startTime_ )
                            , peakTime_( t.peakTime_ )
                            , endTime_( t.endTime_ )
                            , startHeight_( t.startHeight_ )
                            , topHeight_( t.topHeight_ )
                            , endHeight_( t.endHeight_ )
                            , peakArea_( t.peakArea_ )
                            , peakHeight_( t.peakHeight_ )
                            , asymmetry_( t.asymmetry_ )
                            , rs_( t.rs_ )
                            , ntp_( t.ntp_ )
                            , capacityFactor_( t.capacityFactor_ )
                            , peakWidth_( t.peakWidth_ )
                            , peakAmount_( t.peakAmount_ )
                            , migrationTime_( t.migrationTime_ )
                            , peakEfficiency_( t.peakEfficiency_ )
                            , massOnColumn_( t.massOnColumn_ )
                            , percentArea_( t.percentArea_ )
                            , percentHeight_( t.percentHeight_ )
                            , manuallyModified_( t.manuallyModified_ )
                            , tr_( t.tr_ )
{
}

long
Peak::parentId() const
{
    return parentId_;
}

void
Peak::parentId(long id)
{
    parentId_ = id;
}

long
Peak::baseId() const
{
    return baseid_;
}

void
Peak::baseId(long id)
{
    baseid_ = id;
}

long
Peak::peakId() const
{
    return peakid_;
}

void
Peak::peakId(long id)
{
    peakid_ = id;
}

// long UserData() const;
// void UserData(long);

void
Peak::peakFlags(unsigned long v )
{
    peak_flags_ = v;
}

unsigned long
Peak::peakFlags() const
{
    return peak_flags_;
}

const std::wstring&
Peak::name() const
{
    return name_;
}

void
Peak::name( const std::wstring& name )
{
    name_ = name;
}

long
Peak::appliedFunctions() const
{
    return appliedFunctions_;
}

void
Peak::appliedFunctions( long value )
{
    appliedFunctions_ = value;
}

long
Peak::startPos() const
{
    return startPos_;
}

long
Peak::topPos() const
{
    return topPos_;
}

long
Peak::endPos() const
{
    return endPos_;
}

void
Peak::startPos( long pos, peakheight_t h )
{
    startPos_ = pos;
    startHeight_ = h;
}

void
Peak::topPos(long pos,   peakheight_t h)
{
    topPos_ = pos;
    topHeight_ = h;
}

void
Peak::endPos(long pos,   peakheight_t h)
{
    endPos_ = pos;
    endHeight_ = h;
}

seconds_t
Peak::startTime() const
{
    return startTime_;
}

void
Peak::startTime( seconds_t newTime)
{
    startTime_ = newTime;
}

seconds_t
Peak::peakTime() const
{
    return peakTime_;
}

void
Peak::peakTime( seconds_t newTime)
{
    peakTime_ = newTime;
}

seconds_t
Peak::endTime() const
{
    return endTime_;
}

void
Peak::endTime( seconds_t newTime)
{
    endTime_ = newTime;
}

double
Peak::startHeight() const
{
    return startHeight_;
}

double
Peak::topHeight() const
{
    return topHeight_;
}

double
Peak::endHeight() const
{
    return endHeight_;
}

double
Peak::peakArea() const
{
    return peakArea_;
}

void
Peak::peakArea( double value )
{
    peakArea_ = value;
}

double
Peak::peakHeight() const
{
    return peakHeight_;
}

void
Peak::peakHeight( double value )
{
    peakHeight_ = value;
}

double
Peak::capacityFactor() const
{
    return capacityFactor_;
}

void
Peak::capacityFactor( double value )
{
    capacityFactor_ = value;
}

double
Peak::peakWidth() const
{
    return peakWidth_;
}

void
Peak::peakWidth( double value )
{
    peakWidth_ = value;
}

double
Peak::peakAmount() const
{
    return peakAmount_;
}

void
Peak::peakAmount( double value )
{
    peakAmount_ = value;
}

double
Peak::peakEfficiency() const
{
    return peakEfficiency_;
}

void
Peak::peakEfficiency( double value )
{
    peakEfficiency_ = value;
}

double
Peak::percentArea() const
{
    return percentArea_;
}

void
Peak::percentArea( double value )
{
    percentArea_ = value;
}

double
Peak::percentHeight() const
{
    return percentHeight_;
}

void
Peak::percentHeight( double value )
{
    percentHeight_ = value;
}

bool
Peak::isManuallyModified() const
{
    return manuallyModified_;
}

void
Peak::manuallyModified( bool f )
{
    manuallyModified_ = f;
}

const PeakAsymmetry&
Peak::asymmetry() const
{
    return asymmetry_;
}

void
Peak::setAsymmetry( const PeakAsymmetry& t )
{
    asymmetry_ = t;
}

const PeakResolution&
Peak::resolution() const
{
    return rs_;
}

void
Peak::setResolution( const PeakResolution& t )
{
    rs_ = t;
}

const TheoreticalPlate&
Peak::theoreticalPlate() const
{
    return ntp_;
}

void
Peak::setTheoreticalPlate( const TheoreticalPlate& t )
{
    ntp_ = t;
}

const RetentionTime&
Peak::retentionTime() const
{
    return tr_;
}

void
Peak::setRetentionTime( const RetentionTime& tr )
{
    tr_ = tr;
}
