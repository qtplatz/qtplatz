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
PeakResolution::Rs() const
{
    return rs_;
}

double
PeakResolution::RsBaselineStartTime() const
{
    return rsBaselineStartTime_;
}

double
PeakResolution::RsBaselineStartHeight() const
{
    return rsBaselineStartHeight_;
}

double
PeakResolution::RsBaselineEndTime() const
{
    return rsBaselineEndTime_;
}

double
PeakResolution::RsBaselineEndHeight() const
{
    return rsBaselineEndHeight_;
}

double
PeakResolution::RsPeakTopTime() const
{
    return rsPeakTopTime_;
}

double
PeakResolution::RsPeakTopHeight() const
{
    return rsPeakTopHeight_;
}

