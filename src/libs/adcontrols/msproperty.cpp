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
#include "metric/prefix.hpp"
#include "massspectrometer.hpp"
#include "metric/prefix.hpp"
#include <boost/exception/all.hpp>

using namespace adcontrols;

MSProperty::MSProperty() : time_since_injection_( 0 )
                         , instAccelVoltage_( 0 )
                         , tDelay_( 0 )
                         , instNumAvrg_( 0 )
                         , instSamplingStartDelay_( 0 )
                         , instSamplingInterval_( 0 )     
{
}

MSProperty::MSProperty( const MSProperty& t ) : time_since_injection_( t.time_since_injection_ )
                                              , instAccelVoltage_( t.instAccelVoltage_ )
                                              , tDelay_( t.tDelay_ )
                                              , instNumAvrg_( t.instNumAvrg_ )
                                              , instSamplingStartDelay_( t.instSamplingStartDelay_ )
                                              , instSamplingInterval_( t.instSamplingInterval_ )
                                              , dataInterpreterClsid_( t.dataInterpreterClsid_ )
                                              , deviceData_( t.deviceData_ )
                                              , coeffs_( t.coeffs_ ) // depricated
                                              , samplingData_( t.samplingData_ )
{
}

void
MSProperty::setDataInterpreterClsid( const char * utf8 )
{
    dataInterpreterClsid_ = utf8;
}

const char *
MSProperty::dataInterpreterClsid() const
{
    return dataInterpreterClsid_.c_str();
}

void
MSProperty::setDeviceData( const char * device, size_t size )
{
    deviceData_.resize( size );
    std::copy( device, device + size, deviceData_.begin() );
}

const char *
MSProperty::deviceData() const
{
    return deviceData_.data();
}

size_t
MSProperty::deviceDataSize() const
{
    return deviceData_.size();
}

// const std::vector< double >&
// MSProperty::coeffs() const
// {
//     return coeffs_;
// }

// void
// MSProperty::coeffs( const std::vector< double >& coeffs )
// {
//     coeffs_ = coeffs;
// }

double
MSProperty::acceleratorVoltage() const
{
    return instAccelVoltage_;
}

void
MSProperty::acceleratorVoltage( double value )
{
    instAccelVoltage_ = value;
}

double
MSProperty::tDelay() const
{
	return tDelay_;
}

void
MSProperty::tDelay( double t )
{
	tDelay_ = t;
}

int
MSProperty::mode() const
{
    return samplingData_.mode;
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

std::pair<double, double>
MSProperty::instTimeRange() const
{
	const SamplingInfo& x = samplingData_;
    double t0 = metric::scale_to_base( double(x.nSamplingDelay * x.sampInterval), metric::pico );
    double t1 = metric::scale_to_base( double((x.nSamplingDelay + x.nSamples) * x.sampInterval), metric::pico );
    return std::make_pair( t0, t1 );
}

uint32_t
MSProperty::instSamplingInterval() const
{
    return instSamplingInterval_;
}

void
MSProperty::setInstSamplingInterval( uint32_t value )
{
   instSamplingInterval_ = value;
}

uint32_t
MSProperty::instSamplingStartDelay() const
{
    return instSamplingStartDelay_;
}

void
MSProperty::setInstSamplingStartDelay( uint32_t value )
{
    instSamplingStartDelay_ = value;
}


double
MSProperty::timeSinceInjection() const
{
    return metric::scale_to_base<double>( time_since_injection_, metric::micro );
}

void
MSProperty::setTimeSinceInjection( uint32_t value )
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

const MSProperty::SamplingInfo&
MSProperty::getSamplingInfo() const
{
    return samplingData_;
}

void
MSProperty::setSamplingInfo( const SamplingInfo& v )
{
    samplingData_ = v;
}

MSProperty::SamplingInfo::SamplingInfo( uint32_t interval
                                        , uint32_t ndelay
                                        , uint32_t nsamples
                                        , uint32_t navgr
                                        , uint32_t _mode) : sampInterval( interval )
                                                          , nSamplingDelay( ndelay )
                                                          , nSamples( nsamples )  
                                                          , nAverage( navgr )
                                                          , mode( _mode )
{
}
 
MSProperty::SamplingInfo::SamplingInfo() : sampInterval( 0 )
                                         , nSamplingDelay( 0 )
                                         , nSamples( 0 )
                                         , nAverage( 0 )
                                         , mode( 0 )
{
}

//static
double
MSProperty::toSeconds( size_t idx, const SamplingInfo& info )
{
    return ( info.nSamplingDelay + idx ) * info.sampInterval * 1e-12;
}

size_t
MSProperty::compute_profile_time_array( double * p, std::size_t size, const SamplingInfo& info, metric::prefix pfx )
{
    size_t n = 0;
    for ( n = 0; n < size; ++n ) {
		double d = ( info.nSamplingDelay + n ) * info.sampInterval; 
        p[ n ] = metric::scale_to<double>( pfx, d, metric::pico );
	}
    return n;
}

const adcontrols::MassSpectrometer&
MSProperty::spectrometer() const
{
	return adcontrols::MassSpectrometer::get( dataInterpreterClsid() );
}

std::shared_ptr< ScanLaw >
MSProperty::scanLaw() const
{
	try {
		return adcontrols::MassSpectrometer::get( dataInterpreterClsid() ).scanLaw( *this );
	} catch ( boost::exception& e ) {
		e << boost::error_info< struct tag_errmsg, std::string >( "MSProperty::scanLaw" );
		throw;
	}
}

