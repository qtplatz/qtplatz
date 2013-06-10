/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "msproperty.hpp"
#include <boost/foreach.hpp>

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
                                              , samplingData_( t.samplingData_ )
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

unsigned long
MSProperty::instSamplingInterval() const
{
    return instSamplingInterval_;
}

void
MSProperty::setInstSamplingInterval( unsigned long value )
{
   instSamplingInterval_ = value;
}

unsigned long
MSProperty::instSamplingStartDelay() const
{
    return instSamplingStartDelay_;
}

void
MSProperty::setInstSamplingStartDelay( unsigned long value )
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

void
MSProperty::setInstMassRange( const std::pair< double, double >& value )
{
    instMassRange_ = value;
}

const std::pair<double, double>&
MSProperty::instMassRange() const
{
    return instMassRange_;
}

const std::vector< MSProperty::SamplingInfo >&
MSProperty::getSamplingInfo() const
{
    return samplingData_;
}

void
MSProperty::setSamplingInfo( const std::vector< SamplingInfo >& v )
{
    samplingData_ = v;
}

void
MSProperty::addSamplingInfo( const SamplingInfo& item )
{
    samplingData_.push_back( item );
}


MSProperty::SamplingInfo::SamplingInfo( unsigned long interval
                                       , unsigned long ndelay
                                       , unsigned long nsamples
                                       , unsigned long navgr ) : sampInterval( interval )
                                                               , nSamplingDelay( ndelay )
                                                               , nSamples( nsamples )  
                                                               , nAverage( navgr )
{
}
 
MSProperty::SamplingInfo::SamplingInfo() : sampInterval( 0 )
                                         , nSamplingDelay( 0 )
                                         , nSamples( 0 )
                                         , nAverage( 0 )
{
}
 

//static
std::vector<MSProperty::SamplingInfo>::const_iterator
MSProperty::findSamplingInfo( size_t idx, const std::vector<SamplingInfo>& segments )
{
    size_t nSamples = 0;

    std::vector<SamplingInfo>::const_iterator it;
    for ( it = segments.begin(); it != segments.end(); ++it ) {
        nSamples += it->nSamples;
        if ( idx < nSamples )
            break;
    }
    return it;
}

//static
double
MSProperty::toSeconds( size_t idx, const std::vector<SamplingInfo>& segments )
{
    for ( std::vector<SamplingInfo>::const_iterator it = segments.begin(); it != segments.end(); ++it ) {
        if ( idx < it->nSamples )
            return ( it->nSamplingDelay + idx ) * it->sampInterval * 1e-12;
        idx -= it->nSamples;
    }
    return 0.0;
}

//static
double
MSProperty::toSeconds( size_t idx, const SamplingInfo& info )
{
    return ( info.nSamplingDelay + idx ) * info.sampInterval * 1e-12;
}

size_t
MSProperty::compute_profile_time_array( double * p, std::size_t size, const std::vector<SamplingInfo>& segments )
{
    size_t n = 0;
    size_t k = 0;
    std::vector< MSProperty::SamplingInfo >::const_iterator sampInfo = segments.begin();
    for ( ; n < size; ++n, ++k ) {
        if ( k == sampInfo->nSamples ) {
            if ( ++sampInfo == segments.end() )
                break;
            k = 0;
        }
        p[ n ] = ( sampInfo->nSamplingDelay + k ) * sampInfo->sampInterval * 1e-12;
    }
    return n;
}
