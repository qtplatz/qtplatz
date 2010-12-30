//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "peakresolution.h"

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

