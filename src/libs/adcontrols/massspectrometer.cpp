// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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
#include "adcontrols.hpp"
#include "datainterpreter.hpp"
#include "datareader.hpp"
#include "massspectrometerbroker.hpp"
#include "massspectrometer_factory.hpp"
#include "massspectrum.hpp"
#include "mscalibrateresult.hpp"
#include "mscalibration.hpp"
#include "msfractuation.hpp"
#include "msproperty.hpp"
#include "mspeak.hpp"
#include "mspeaks.hpp"
#include "scanlaw.hpp"
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/polfit.hpp>
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
    }
}

///////////////////////////////////////////////
MassSpectrometer::MassSpectrometer() : datafile_(0)
                                     , acceleratorVoltage_( 5000 )
                                     , tDelay_( 0 )
                                     , fLength_( 1.0 )
{
}

MassSpectrometer::MassSpectrometer( adcontrols::datafile * datafile ) : datafile_( datafile )
                                                                      , acceleratorVoltage_( 5000 )
                                                                      , tDelay_( 0 )
                                                                      , fLength_( 1.0 )
{
}

MassSpectrometer::~MassSpectrometer()
{
}

void
MassSpectrometer::setAcceleratorVoltage( double acclVolts, double tDelay )
{
    acceleratorVoltage_ = acclVolts;
    tDelay_ = tDelay;
}

// void
// MassSpectrometer::setScanLaw( double acclVolts, double tDelay, double fLength )
// {
//     acceleratorVoltage_ = acclVolts;
//     tDelay_  = tDelay;
//     fLength_ = fLength;
// }

double
MassSpectrometer::acceleratorVoltage() const
{
    return acceleratorVoltage_;
}

double
MassSpectrometer::fLength() const
{
    return fLength_;
}

double
MassSpectrometer::tDelay() const
{
    return tDelay_;
}

const wchar_t *
MassSpectrometer::name() const
{
    return 0;
}

std::shared_ptr<ScanLaw>
MassSpectrometer::scanLaw( const adcontrols::MSProperty& prop ) const
{
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

// v3
bool
MassSpectrometer::assignMasses( MassSpectrum& ms ) const
{
    auto mode = ms.mode();
    return ms.assign_masses( [&]( double time, int mode ) { return scanLaw()->getMass( time, mode ); } );
}

void
MassSpectrometer::setDataReader( DataReader * reader )
{
    reader_ = reader->shared_from_this();
}

void
MassSpectrometer::setMSFractuation( MSFractuation * fractuation )
{
    msfractuation_ = fractuation->shared_from_this();
}

MSFractuation *
MassSpectrometer::msFractuation() const
{
    return msfractuation_.get();
}

// v2 interface
adcontrols::datafile *
MassSpectrometer::datafile() const
{
    return datafile_;
}

void
MassSpectrometer::setDebugTrace( const char * logfile, int level )
{
}

// create
std::shared_ptr< adcontrols::MassSpectrometer >
MassSpectrometer::create( const char * dataInterpreterClsid )
{
    return MassSpectrometerBroker::make_massspectrometer( dataInterpreterClsid );
}

//static
std::shared_ptr< ScanLaw >
MassSpectrometer::make_scanlaw( const adcontrols::MSProperty& prop )
{
    if ( auto spectrometer = create( prop.dataInterpreterClsid() ) ) {

        return spectrometer->scanLaw( prop );
        
    }
    return nullptr;
}

std::vector< std::wstring > 
MassSpectrometer::get_model_names()
{
    std::vector< std::wstring > names;
    auto list = MassSpectrometerBroker::installed_uuids();
    std::for_each( list.begin(), list.end(), [&] ( const std::pair< boost::uuids::uuid, std::string >& a ) {
            names.push_back( adportable::utf::to_wstring( a.second ) ); } );
    return names;
}

double
MassSpectrometer::timeFromMass( double mass, const MassSpectrum& ms ) const
{
    return scanLaw()->getTime( mass, ms.mode() );
}

double 
MassSpectrometer::massFromTime( double time, const MassSpectrum& ms ) const
{
    return scanLaw()->getMass( time, ms.mode() );
}

std::pair<double,double>
MassSpectrometer::timeFromMass( const std::pair<double,double>& range, const MassSpectrum& ms ) const
{
    return std::make_pair( timeFromMass( range.first, ms ), timeFromMass( range.second, ms ) );        
}

std::pair<double,double>
MassSpectrometer::massFromTime( const std::pair<double,double>& range, const MassSpectrum& ms ) const
{
    return std::make_pair( massFromTime( range.first, ms ), massFromTime( range.second, ms ) );
}

//////////////////////////////////////////////////////////////
bool
MassSpectrometer::estimateScanLaw( const adcontrols::MSPeaks& pks, double& va, double& t0 ) const
{
    std::vector< std::tuple< double, double, int > > peaks;
    for ( auto& pk: pks )
        peaks.emplace_back( pk.time(), pk.exact_mass(), pk.mode() );
    return estimateScanLaw( peaks, va, t0 );
}

bool
MassSpectrometer::estimateScanLaw( const std::vector< std::tuple< double, double, int > >& peaks // mass, time, mode
                                   , double& va
                                   , double& t0 ) const
{
    using namespace adcontrols::metric;
    
    if ( auto law = scanLaw() ) {

        if ( peaks.size() == 1 ) {

            auto& pk = peaks[ 0 ];
            t0 = 0.0;
            va = adportable::TimeSquaredScanLaw::acceleratorVoltage( std::get< 0 >( pk ), std::get< 1 >( pk ), law->fLength( std::get< 2 >( pk ) ), t0 );
            return true;

        } else if ( peaks.size() >= 2 ) {
        
            std::vector<double> x, y, coeffs;

            for ( auto& pk : peaks ) {
                x.push_back( std::sqrt( std::get<0>( pk ) ) * law->fLength( std::get<2>(pk) ) );  // exact_mass * fLength(mode)
                y.push_back( std::get< 1 >( pk ) ); // time
            }

            if ( adportable::polfit::fit( x.data(), y.data(), x.size(), 2, coeffs ) ) {
                
                t0 = coeffs[ 0 ];
                double t1 = adportable::polfit::estimate_y( coeffs, 1.0 ); // estimate tof for m/z = 1.0, fLength = 1m
                va = adportable::TimeSquaredScanLaw::acceleratorVoltage( 1.0, t1, 1.0, t0 );
                
                return true;
            }
        }
    }
    return false;
}
