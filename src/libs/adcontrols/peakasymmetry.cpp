//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "peakasymmetry.h"

using namespace adcontrols;

PeakAsymmetry::PeakAsymmetry() : peakAsymmetry_(0)
                               , peakAsymmetryStartTime_(0)
                               , peakAsymmetryEndTime_(0)
{
}

PeakAsymmetry::PeakAsymmetry( const PeakAsymmetry& t ) : peakAsymmetry_( t.peakAsymmetry_ )
                                                       , peakAsymmetryStartTime_( t.peakAsymmetryStartTime_ )
                                                       , peakAsymmetryEndTime_( t.peakAsymmetryEndTime_ )
{
}

double
PeakAsymmetry::asymmetry() const
{
    return peakAsymmetry_;
}

double
PeakAsymmetry::startTime() const
{
    return peakAsymmetryStartTime_;
}

double
PeakAsymmetry::endTime() const
{
    return peakAsymmetryEndTime_;
}
