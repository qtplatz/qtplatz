// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#include "massspectrometer.hpp"
#include "datainterpreter.hpp"
#include "massspectrometerbroker.hpp"
#include "massspectrometer_factory.hpp"
#include "adcontrols.hpp"
#include "mscalibrateresult.hpp"
#include <adportable/utf.hpp>
#include <string>
#include <cmath>

using namespace adcontrols;

namespace adcontrols {

    namespace internal {

        class DataInterpreter : public adcontrols::DataInterpreter {
        public:
            DataInterpreter(void) {  }
            virtual ~DataInterpreter(void) {  }

            adcontrols::translate_state translate( MassSpectrum&
                                                   , const char * data, size_t dsize
                                                   , const char * meta, size_t msize 
                                                   , const MassSpectrometer&
                                                   , size_t idData
												   , const MSCalibration * ) const override {
                (void)data; (void)dsize; (void)meta; (void)msize; (void)idData;
                return adcontrols::translate_error;
            }
            
            adcontrols::translate_state translate( TraceAccessor&
                                                   , const char * data, size_t dsize
                                                   , const char * meta, size_t msize
                                                   , unsigned long events ) const override {
                (void)data; (void)dsize; (void)meta; (void)msize; (void)events;
                return adcontrols::translate_error;
            }

        };

    }
}

TimeSquaredScanLaw::TimeSquaredScanLaw( double kAcceleratorVoltage
                                        , double tDelay
                                        , double fLength )
    : kAcceleratorVoltage_( kAcceleratorVoltage )
    , tDelay_( tDelay )
	, kTimeSquaredCoeffs_( 2.0 * kELEMENTAL_CHARGE / kATOMIC_MASS_CONSTANT )
    , fLength_( fLength )
{
}

TimeSquaredScanLaw::TimeSquaredScanLaw( const TimeSquaredScanLaw& t )
    : kAcceleratorVoltage_( t.kAcceleratorVoltage_ )
    , tDelay_( t.tDelay_ )
    , kTimeSquaredCoeffs_( t.kTimeSquaredCoeffs_ )
    , fLength_( t.fLength_ )
{
}

double
TimeSquaredScanLaw::getMass( double time, int mode ) const
{
    return getMass( time, fLength( mode ) );
}

double
TimeSquaredScanLaw::getTime( double mass, int mode ) const
{
    return getTime( mass, fLength( mode ) );
}

double
TimeSquaredScanLaw::getMass( double time, double fLength ) const
{
    double k = ( kTimeSquaredCoeffs * kAcceleratorVoltage_ ) / ( fLength * fLength );
    double t = time - tDelay_;
    double mass = k * ( t * t );
	return mass;
}

double
TimeSquaredScanLaw::getTime( double mass, double fLength ) const
{
    double k = ( kTimeSquaredCoeffs * kAcceleratorVoltage_ ) / ( fLength * fLength );
	double time = std::sqrt(mass / k) + tDelay_;
	return time;
}

double
TimeSquaredScanLaw::fLength( int mode ) const
{
    // This method should be orverridden for MULTITURN spectrometer or more than 2 mode
    // analyzer spectrometer (ex. Linear|Refrectron mode for MALDI TOF spectrometer)
	(void)mode;
    return fLength_; 
}

///////////////////////////////////////////////
MassSpectrometer::MassSpectrometer() : instance_( 0 )
                                     , scanLaw_( std::make_shared< TimeSquaredScanLaw >())
{
}

MassSpectrometer::MassSpectrometer( const MassSpectrometer& t ) : instance_( t.instance_ )
                                                                , scanLaw_( t.scanLaw_ )
{
}

MassSpectrometer::~MassSpectrometer()
{
}

const wchar_t *
MassSpectrometer::name() const
{
    if ( instance_ )
        return instance_->name();
    return L"default";
}

const ScanLaw&
MassSpectrometer::getScanLaw() const
{
    if ( instance_ )
        return instance_->getScanLaw();
    return *scanLaw_;
}

const DataInterpreter&
MassSpectrometer::getDataInterpreter() const
{
    if ( instance_ )
        return instance_->getDataInterpreter();

    static internal::DataInterpreter t;
    return t;
}

std::shared_ptr<ScanLaw>
MassSpectrometer::scanLaw( const adcontrols::MSProperty& prop ) const
{
    if ( instance_ ) 
        return instance_->scanLaw( prop );
    return scanLaw_;
}

void
MassSpectrometer::setCalibration( int mode, const MSCalibrateResult& cr )
{
    mode_calib_map_[ mode ] = std::make_shared< MSCalibrateResult >( cr );
}

const std::shared_ptr< adcontrols::MSCalibrateResult >
MassSpectrometer::findCalibration( int mode ) const
{
    auto it = mode_calib_map_.find( mode );
    if ( it != mode_calib_map_.end() )
        return it->second;
    return 0;
}

const std::shared_ptr< adcontrols::MSCalibrateResult >
MassSpectrometer::calibration( size_t idx ) const
{
    if ( idx < mode_calib_map_.size() ) {
        auto it = mode_calib_map_.begin();
        std::advance( it, idx );
        return it->second;
    }
    return 0;
}

std::shared_ptr< MassSpectrometer >
MassSpectrometer::create( const wchar_t * dataInterpreterClsid )
{
	if ( massspectrometer_factory * factory = massSpectrometerBroker::find( dataInterpreterClsid ) ) {
        if ( auto ptr = std::make_shared< MassSpectrometer >() ) {
            if ( ptr->instance_ = factory->get( dataInterpreterClsid ) )
                return ptr;
        }
    }
    return 0;
}

const MassSpectrometer&
MassSpectrometer::get( const wchar_t * dataInterpreterClsid )
{
	massspectrometer_factory * factory = massSpectrometerBroker::find( dataInterpreterClsid );
	if ( factory )
		return *factory->get( dataInterpreterClsid );
	throw std::bad_cast( "can't find specified spectrometer" );
}

const MassSpectrometer&
MassSpectrometer::get( const char * dataInterpreterClsid )
{
	std::wstring wstr = adportable::utf::to_wstring( dataInterpreterClsid );
	return get( wstr.c_str() );
}

std::vector< std::wstring > 
MassSpectrometer::get_model_names()
{
    return massSpectrometerBroker::names();
}

//////////////////////////////////////////////////////////////
