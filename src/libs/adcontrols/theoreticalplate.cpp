//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "theoreticalplate.h"

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
