//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "msproperty.h"

using namespace adcontrols;

MSProperty::MSProperty() : time_since_injection_( 0 )
                         , instAccelVoltage_( 0 )
                         , instNumAvrg_( 0 )
                         , instSamplingStartDelay_( 0 )
                         , instSamplingInterval_( 0 )     
{
}

MSProperty::MSProperty( const MSProperty& t ) : time_since_injection_( t.time_since_injection_ )
                                              , instAccelVoltage_( t.instAccelVoltage_ )
                                              , instNumAvrg_( t.instNumAvrg_ )
                                              , instSamplingStartDelay_( t.instSamplingStartDelay_ )
                                              , instSamplingInterval_( t.instSamplingInterval_ )     
{
}

double
MSProperty::accelerateVoltage() const
{
    return instAccelVoltage_;
}

void
MSProperty::setAccelerateVoltage( double value )
{
    instAccelVoltage_ = value;
}


// number of average for waveform
size_t
MSProperty::numAverage() const
{
    return instNumAvrg_;
}

void
MSProperty::setNumAverage( size_t value )
{
    instNumAvrg_ = value;
}

double
MSProperty::time( size_t pos ) // return flight time for data[pos] in seconds
{
    return double( instSamplingStartDelay_ + pos ) * instSamplingInterval_ * 1.0e12;  // ps -> s
}

size_t
MSProperty::instSamplingInterval() const
{
    return instSamplingInterval_;
}

void
MSProperty::setInstSamplingInterval( size_t value )
{
   instSamplingInterval_ = value;
}

size_t
MSProperty::instSamplingStartDelay() const
{
    return instSamplingStartDelay_;
}

void
MSProperty::setInstSamplingStartDelay( size_t value )
{
    instSamplingStartDelay_ = value;
}


unsigned long
MSProperty::timeSinceInjection() const
{
    return time_since_injection_;
}

void
MSProperty::setTimeSinceInjection( unsigned long value )
{
    time_since_injection_ = value;
}

