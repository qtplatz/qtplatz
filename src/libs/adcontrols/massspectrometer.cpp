// -*- C++ -*-
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

#include "massspectrometer.hpp"
#include "scanlaw.hpp"
#include "datainterpreter.hpp"
#include "massspectrometerbroker.hpp"
#include "massspectrometer_factory.hpp"
#include "adcontrols.hpp"
#include "mscalibrateresult.hpp"
#include "mscalibration.hpp"
#include <adportable/timesquaredscanlaw.hpp>
#include <adportable/utf.hpp>
#include <boost/exception/all.hpp>
#include <string>
#include <cmath>

using namespace adcontrols;

namespace adcontrols {

    ScanLaw::ScanLaw()
    {
    }
    
    ScanLaw::~ScanLaw()
    {
    }

	class MassSpectrometerException : public boost::exception, public std::exception {
	public:
		MassSpectrometerException( const std::string& msg ) {
			*this << boost::error_info< struct tag_errmsg, std::string >( msg );
		}
	};

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
												   , const wchar_t * traceId ) const override {
                (void)data; (void)dsize; (void)meta; (void)msize; (void)idData; (void)traceId;
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

        class GenericTofSpectrometer : public adcontrols::MassSpectrometer
                                     , public adcontrols::ScanLaw
                                     , public adcontrols::massspectrometer_factory {

            adportable::TimeSquaredScanLaw law_;
            DataInterpreter interpreter_;
        public:
            GenericTofSpectrometer() {}
            GenericTofSpectrometer( const GenericTofSpectrometer& t ) : law_( t.law_ ) {}

            // adcontrols::ScanLaw
            double getMass( double t, int mode ) const override { return law_.getMass( t, mode );  }
            double getTime( double m, int mode ) const override { return law_.getTime( m, mode );  }
            double getMass( double t, double fLength ) const override { return law_.getMass( t, fLength ); }
            double getTime( double m, double fLength ) const override { return law_.getTime( m, fLength ); }
            double fLength( int type ) const override { return law_.fLength( type ); }            

            // MassSpectrometer
            const adcontrols::ScanLaw& getScanLaw() const override { return *this; }
            std::shared_ptr< adcontrols::ScanLaw > scanLaw( const adcontrols::MSProperty& ) const override {
                return std::make_shared< GenericTofSpectrometer >(*this);
            }
            const adcontrols::DataInterpreter& getDataInterpreter() const override { return interpreter_; }

            // massspectrometer_factory
            const wchar_t * name() const override { return L"Generic-TOF"; }
            adcontrols::MassSpectrometer * get( const wchar_t * modelname ) override { return this; } // depricated
            std::shared_ptr< MassSpectrometer > create( const wchar_t * modelname, adcontrols::datafile * file ) const override {
                return std::make_shared< GenericTofSpectrometer >();
            }
        };

    }
}



///////////////////////////////////////////////
MassSpectrometer::MassSpectrometer() : proxy_instance_( 0 )
                                     , datafile_(0)
{
}

MassSpectrometer::MassSpectrometer( adcontrols::datafile * datafile ) : proxy_instance_( 0 )
                                                                      , datafile_( datafile )
{
}

MassSpectrometer::~MassSpectrometer()
{
}

const wchar_t *
MassSpectrometer::name() const
{
    if ( proxy_instance_ )
        return proxy_instance_->name();
    return 0;
}

const ScanLaw&
MassSpectrometer::getScanLaw() const
{
    if ( proxy_instance_ )
        return proxy_instance_->getScanLaw();

    throw std::bad_cast();
}

const DataInterpreter&
MassSpectrometer::getDataInterpreter() const
{
    if ( proxy_instance_ )
        return proxy_instance_->getDataInterpreter();

    static internal::DataInterpreter t;
    return t;
}

std::shared_ptr<ScanLaw>
MassSpectrometer::scanLaw( const adcontrols::MSProperty& prop ) const
{
    if ( proxy_instance_ ) 
        return proxy_instance_->scanLaw( prop );
    return 0;
}

void
MassSpectrometer::setCalibration( int mode, const MSCalibrateResult& cr )
{
    auto ptr = std::make_shared< MSCalibrateResult >( cr );
    mode_calib_map_[ mode ] = 0;
    mode_calib_map_[ mode ] = ptr;
}

const adcontrols::MSCalibration *
MassSpectrometer::findCalibration( int mode ) const
{
    auto it = mode_calib_map_.find( mode );
    if ( it != mode_calib_map_.end() )
        return &it->second->calibration();
    return 0;
}

const std::shared_ptr< adcontrols::MSCalibrateResult >
MassSpectrometer::getCalibrateResult( size_t idx ) const
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
        return factory->create( dataInterpreterClsid, 0 );
    }
    return 0;
}

std::shared_ptr< MassSpectrometer >
MassSpectrometer::create( const char * dataInterpreterClsid )
{
    return create( adportable::utf::to_wstring( dataInterpreterClsid ).c_str() );
}

std::shared_ptr< MassSpectrometer >
MassSpectrometer::create( const wchar_t * dataInterpreterClsid, adcontrols::datafile * datafile )
{
	if ( massspectrometer_factory * factory = massSpectrometerBroker::find( dataInterpreterClsid ) )
        return factory->create( dataInterpreterClsid, datafile );
    return 0;
}

adcontrols::datafile *
MassSpectrometer::datafile() const
{
    return datafile_;
}

void
MassSpectrometer::setDebugTrace( const char * logfile, int level )
{
}

const MassSpectrometer*
MassSpectrometer::find( const wchar_t * dataInterpreterClsid )
{
	massspectrometer_factory * factory = massSpectrometerBroker::find( dataInterpreterClsid );
	if ( factory )
		return factory->get( dataInterpreterClsid );
	return 0;
}

const MassSpectrometer*
MassSpectrometer::find( const char * dataInterpreterClsid )
{
	return find( adportable::utf::to_wstring( dataInterpreterClsid ).c_str() );
}

const MassSpectrometer&
MassSpectrometer::get( const wchar_t * dataInterpreterClsid )
{
	massspectrometer_factory * factory = massSpectrometerBroker::find( dataInterpreterClsid );
	if ( factory )
		return *factory->get( dataInterpreterClsid );
    std::ostringstream o;
    o << "Data interpreter: \"" << adportable::utf::to_utf8( dataInterpreterClsid ) << "\" does not installed";
	BOOST_THROW_EXCEPTION( MassSpectrometerException( o.str() ) );
}

const MassSpectrometer&
MassSpectrometer::get( const char * dataInterpreterClsid )
{
	std::wstring clsid = adportable::utf::to_wstring( dataInterpreterClsid );
	return get( clsid.c_str() );
}

std::vector< std::wstring > 
MassSpectrometer::get_model_names()
{
    return massSpectrometerBroker::names();
}

void
MassSpectrometer::register_default_spectrometers()
{
    static adcontrols::internal::GenericTofSpectrometer * tof = new adcontrols::internal::GenericTofSpectrometer();
    massSpectrometerBroker::register_factory( tof, tof->name() );
}

//////////////////////////////////////////////////////////////
