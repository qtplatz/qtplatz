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

TheoreticalPlate::TheoreticalPlate( const TheoreticalPlate& t ) : ntp_( t.ntp_ )
                                                                , ntpBaselineStartTime_( t.ntpBaselineStartTime_ )
                                                                , ntpBaselineStartHeight_( t.ntpBaselineStartHeight_ )
                                                                , ntpBaselineEndTime_( t.ntpBaselineEndTime_ )
                                                                , ntpBaselineEndHeight_( t.ntpBaselineEndHeight_ )
                                                                , ntpPeakTopTime_( t.ntpPeakTopTime_ )
                                                                , ntpPeakTopHeight_( t.ntpPeakTopHeight_ )
{
}

double
TheoreticalPlate::NTP() const
{
    return ntp_;
}

double
TheoreticalPlate::NTPBaselineStartTime() const
{
    return ntpBaselineStartTime_;
}

double
TheoreticalPlate::NTPBaselineStartHeight() const
{
    return ntpBaselineStartHeight_;
}

double
TheoreticalPlate::NTPBaselineEndTime() const
{
    return ntpBaselineEndTime_;
}

double
TheoreticalPlate::NTPBaselineEndHeight() const
{
    return ntpBaselineEndHeight_;
}

double
TheoreticalPlate::NTPPeakTopTime() const
{
    return ntpPeakTopTime_;
}

double
TheoreticalPlate::NTPPeakTopHeight() const
{
    return ntpPeakTopHeight_;
}
