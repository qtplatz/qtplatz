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

#include "rawdata.hpp"
#include <adinterface/signalobserver.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/traceaccessor.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adfs/adfs.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/serializer.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adlog/logger.hpp>
#include <adutils/mscalibio.hpp>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <memory>
#include <cstdint>
#include <sstream>

using namespace addatafile;

rawdata::~rawdata()
{
}

rawdata::rawdata( adfs::filesystem& dbf
                  , adcontrols::datafile& parent ) : dbf_( dbf )
                                                   , parent_( parent )
                                                   , npos0_( 0 )
                                                   , configLoaded_( false )
{
}

adfs::sqlite *
rawdata::db()
{
    return &dbf_.db();
}

bool
rawdata::loadAcquiredConf()
{
    if ( configLoaded_ )
        return true;

    conf_.clear();

    if ( adutils::AcquiredConf::fetch( dbf_.db(), conf_ ) && !conf_.empty() ) {

        for ( const auto& conf: conf_ ) {
            
            if ( conf.trace_method == signalobserver::eTRACE_TRACE // timed trace := chromatogram
                 && conf.trace_id == L"MS.TIC" ) {
                
                adcontrols::TraceAccessor accessor;
                if ( fetchTraces( conf.objid, conf.dataInterpreterClsid, accessor ) ) {
                    for ( int fcn = 0; fcn < accessor.nfcn(); ++fcn ) {
                        std::shared_ptr< adcontrols::Chromatogram > cptr( new adcontrols::Chromatogram() );
                        cptr->addDescription( adcontrols::Description( L"create",  conf.trace_display_name ) );
                        accessor.copy_to( *cptr, fcn );
                        tic_.push_back( cptr );
                    }
            }
            }
        }
        configLoaded_ = true;
        return true;
    }
    return false;
}

bool
rawdata::applyCalibration( const std::wstring& dataInterpreterClsid, const adcontrols::MSCalibrateResult& calibResult )
{
    uint64_t objid = 1;
    
    auto it = std::find_if( conf_.begin(), conf_.end(), [=](const adutils::AcquiredConf::data& c){
            return c.dataInterpreterClsid == dataInterpreterClsid;
        });

    if ( it != conf_.end() )
        objid = it->objid;
    else {
        if ( conf_.empty() ) 
            adutils::AcquiredConf::create_table( dbf_.db() );
		adutils::AcquiredConf::data d;
		d.objid = objid;
		d.pobjid = 0;
		d.dataInterpreterClsid = dataInterpreterClsid;
		if ( ! adutils::AcquiredConf::insert( dbf_.db(), d ) )
            return false;
    }

	const std::wstring calibId = calibResult.calibration().calibId();
    std::string device;
    if ( adportable::serializer< adcontrols::MSCalibrateResult >::serialize( calibResult, device ) ) {
        adutils::mscalibio::writeCalibration( dbf_.db(), uint32_t( objid ), calibId.c_str(), calibResult.dataClass(), device.data(), device.size() );
        loadAcquiredConf();
        loadCalibrations();
        return true;
    }
    return false;
}

void
rawdata::loadCalibrations()
{
    using adportable::serializer;
    using adcontrols::MSCalibrateResult;

    std::for_each( conf_.begin(), conf_.end(), [&]( const adutils::AcquiredConf::data& conf ){
            std::vector< char > device;
            int64_t rev;
            if ( adutils::mscalibio::readCalibration( dbf_.db(), uint32_t(conf.objid), MSCalibrateResult::dataClass(), device, rev ) ) {
                auto calibResult = std::make_shared< MSCalibrateResult >();
                if ( serializer< MSCalibrateResult >::deserialize( *calibResult, device.data(), device.size() ) ) {
                    calibResults_[ conf.objid ] = calibResult;
					auto spectrometer = getSpectrometer( conf.objid, conf.dataInterpreterClsid.c_str() );
					spectrometer.setCalibration( calibResult->mode(), *calibResult );
                }
            }
        });
}

const adcontrols::MassSpectrometer&
rawdata::getSpectrometer( uint64_t objid, const std::wstring& dataInterpreterClsid ) const
{
    auto it = spectrometers_.find( objid );
    if ( it == spectrometers_.end() ) {
		if ( auto ptr = adcontrols::MassSpectrometer::create( dataInterpreterClsid.c_str(), &parent_ ) ) {
			const_cast< rawdata& >(*this).spectrometers_[ objid ] = ptr;
			it = spectrometers_.find( objid );
		} else {
			static adcontrols::MassSpectrometer x;
			return x;
		}
    }
    return *it->second;
}

size_t
rawdata::getSpectrumCount( int fcn ) const
{
	auto it = std::find_if( conf_.begin(), conf_.end(), []( const adutils::AcquiredConf::data& c ){
            return c.trace_method == signalobserver::eTRACE_SPECTRA && c.trace_id == L"MS.PROFILE";
        });
    if ( it != conf_.end() ) {
        adfs::stmt sql( dbf_.db() );
        if ( sql.prepare( "SELECT count(rowid) FROM AcquiredData WHERE oid = :oid AND fcn = :fcn" ) ) {
            sql.bind( 1 ) = it->objid;
            sql.bind( 2 ) = fcn;
            if ( sql.step() == adfs::sqlite_row )
                return static_cast< size_t >( sql.get_column_value<int64_t>( 0 ) );
        }
    }
    return 0;
}

bool
rawdata::getSpectrum( int fcn, int idx, adcontrols::MassSpectrum& ms, uint32_t objid ) const
{
    auto it = std::find_if( conf_.begin(), conf_.end(), [=]( const adutils::AcquiredConf::data& c ){
            if ( objid == 0 )
                return c.trace_method == signalobserver::eTRACE_SPECTRA && c.trace_id == L"MS.PROFILE";
            else
                return c.objid == objid;
        });
    if ( it == conf_.end() )
        return false;

    uint64_t npos = npos0_ + idx;

    adfs::stmt sql( dbf_.db() );
    if ( sql.prepare( "SELECT max(npos) FROM AcquiredData WHERE oid = :oid AND fcn = :fcn AND npos <= :npos" ) ) {
        sql.bind( 1 ) = it->objid;
        sql.bind( 2 ) = fcn;
        sql.bind( 3 ) = npos;
        if ( sql.step() == adfs::sqlite_row ) {
            try {
                npos = sql.get_column_value< uint64_t >( 0 );
            } catch ( std::bad_cast& ) {
                return false; // max() function returns nil raw as step() result, so this exception means no row
            }
        }
    }
    adcontrols::translate_state state;
	while ( ( state = fetchSpectrum( it->objid, it->dataInterpreterClsid, npos++, ms, it->trace_id ) )
            == adcontrols::translate_indeterminate )
        ;
    return state == adcontrols::translate_complete;
}

bool
rawdata::getTIC( int fcn, adcontrols::Chromatogram& c ) const
{
    if ( tic_.size() > unsigned( fcn ) ) {
        c = *tic_[ fcn ];
        return true;
    }
    return false;
}

size_t
rawdata::getChromatogramCount() const
{
    return 0;
}

size_t
rawdata::getFunctionCount() const
{
    return tic_.size();
}

size_t
rawdata::posFromTime( double seconds ) const
{
	if ( ! tic_.empty() ) {
		const adportable::array_wrapper< const double > times( tic_[0]->getTimeArray(), tic_[0]->size() );
		auto it = std::lower_bound( times.begin(), times.end(), seconds ); 
		
		return std::distance( times.begin(), it );
	}
	return 0;
}

double
rawdata::timeFromPos( size_t pos ) const
{
	if ( !tic_.empty() && pos < tic_[0]->size() ) {
		return tic_[0]->getTimeArray()[ pos ]; // return in seconds
	}
	return 0;
}

bool
rawdata::getChromatograms( const std::vector< std::tuple<int, double, double> >& ranges
                           , std::vector< adcontrols::Chromatogram >& result
                           , std::function< bool (long curr, long total ) > progress
                           , int begPos
                           , int endPos ) const
{
    result.clear();

	auto it = std::find_if( conf_.begin(), conf_.end(), []( const adutils::AcquiredConf::data& c ){
            return c.trace_method == signalobserver::eTRACE_SPECTRA && c.trace_id == L"MS.PROFILE";
        });
    
	if ( it == conf_.end() )
        return false;

    size_t spCount = getSpectrumCount( 0 );
    if ( endPos < 0 || endPos >= int( spCount ) )
        endPos = spCount - 1;

    size_t nData = endPos - begPos + 1;
    uint64_t npos = npos0_ + begPos;

    adfs::stmt sql( dbf_.db() );
    if ( sql.prepare( "SELECT max(npos) FROM AcquiredData WHERE oid = :oid AND fcn = 0 AND npos <= :npos" ) ) {
        sql.bind( 1 ) = it->objid;
        sql.bind( 2 ) = npos;
        if ( sql.step() == adfs::sqlite_row )
            npos = sql.get_column_value< int64_t >( 0 );
    }

    std::vector< std::tuple< int, double, double > > xranges( ranges );

    for ( auto& range: xranges ) {
        adcontrols::Chromatogram c;
        c.resize( nData );
        c.setTimeArray( tic_[0]->getTimeArray() + begPos );
        double lMass = std::get<1>( range );
        double uMass = std::get<2>( range );
        std::wostringstream o;
        if ( uMass < 1.0 ) {
            o << boost::wformat( L"m/z %.4lf (W:%.4lfDa) %d" ) % std::get<1>(range) % std::get<2>(range) % std::get<0>(range);
            range = std::make_tuple( std::get<0>(range), lMass - uMass / 2, lMass + uMass / 2 );
        } else {
            o << boost::wformat( L"m/z (%.4lf - %.4lf) %d" ) % std::get<1>(range) % lMass % uMass % std::get<0>(range);
        }
        c.addDescription( adcontrols::Description( L"Create", o.str() ) );            
        result.push_back( c );
    }

    std::vector< adportable::spectrum_processor::areaFraction > fractions;

    for ( size_t i = 0; i < nData; ++i ) {

        progress( i, nData );

        adcontrols::MassSpectrum ms;
        adcontrols::translate_state state;
		while ( ( state = fetchSpectrum( it->objid, it->dataInterpreterClsid, npos++, ms, it->trace_id ) )
                == adcontrols::translate_indeterminate )
            ;

        size_t nch = 0;
        for ( auto& range: xranges ) {
            int fcn = std::get<0>(range);
            double lMass = std::get<1>(range);
            double uMass = std::get<2>(range);
            const adcontrols::MassSpectrum& fms = fcn == 0 ? ms : ms.getSegment( fcn );
            if ( i == 0 ) {
                adportable::spectrum_processor::areaFraction fraction;
				adportable::spectrum_processor::getFraction( fraction, fms.getMassArray(), fms.size(), lMass, uMass );
                fractions.push_back( fraction );
            }
            const adportable::spectrum_processor::areaFraction& frac = fractions[ nch ];
            double base(0), rms(0);
			double tic = adportable::spectrum_processor::tic( fms.size(), fms.getIntensityArray(), base, rms );
			(void)tic;
			(void)rms;
			double d = adportable::spectrum_processor::area( frac, base, fms.getIntensityArray(), fms.size() );

            result[ nch++ ].setIntensity( i, d );
        }
    }

    return true;
}

// private
bool
rawdata::fetchTraces( int64_t objid, const std::wstring& dataInterpreterClsid, adcontrols::TraceAccessor& accessor )
{
    const adcontrols::MassSpectrometer& spectrometer = getSpectrometer( objid, dataInterpreterClsid.c_str() );
    const adcontrols::DataInterpreter& interpreter = spectrometer.getDataInterpreter();

    adfs::stmt sql( dbf_.db() );
    
    if ( sql.prepare( "SELECT rowid, npos, events, fcn FROM AcquiredData WHERE oid = :oid ORDER BY npos" ) ) {

        sql.bind( 1 ) = objid;
        adfs::blob blob;
        std::vector< char > xdata;
        std::vector< char > xmeta;
		size_t nrecord = 0;

        while( sql.step() == adfs::sqlite_row ) {
			++nrecord;
            uint64_t rowid = sql.get_column_value< int64_t >( 0 );
            uint64_t npos  = sql.get_column_value< int64_t >( 1 );
            uint32_t events = static_cast< uint32_t >( sql.get_column_value< int64_t >( 2 ) );
            if ( npos0_ == 0 )
                npos0_ = npos;

            if ( blob.open( dbf_.db(), "main", "AcquiredData", "data", rowid, adfs::readonly ) ) {
                xdata.resize( blob.size() );
                if ( blob.size() )
                    blob.read( reinterpret_cast< int8_t *>( xdata.data() ), blob.size() );
            }

            if ( blob.open( dbf_.db(), "main", "AcquiredData", "meta", rowid, adfs::readonly ) ) {
                xmeta.resize( blob.size() );
                if ( blob.size() )
                    blob.read( reinterpret_cast< int8_t *>( xmeta.data() ), blob.size() );
            }

            interpreter.translate( accessor, xdata.data(), xdata.size(), xmeta.data(), xmeta.size()
                                   , static_cast< uint32_t >( events ) );
        }
		
		return nrecord != 0;
    }

    return false;
}

// private
adcontrols::translate_state
rawdata::fetchSpectrum( int64_t objid
                        , const std::wstring& dataInterpreterClsid
                        , uint64_t npos, adcontrols::MassSpectrum& ms
						, const std::wstring& traceId ) const
{
    const adcontrols::MassSpectrometer& spectrometer = getSpectrometer( objid, dataInterpreterClsid );
    const adcontrols::DataInterpreter& interpreter = spectrometer.getDataInterpreter();

    adfs::stmt sql( dbf_.db() );

    if ( sql.prepare( "SELECT rowid, fcn FROM AcquiredData WHERE oid = :oid AND npos = :npos" ) ) {

        sql.bind( 1 ) = objid;
        sql.bind( 2 ) = npos;

        adfs::blob blob;
        std::vector< char > xdata;
        std::vector< char > xmeta;

        if ( sql.step() == adfs::sqlite_row ) {
			uint64_t rowid  = sql.get_column_value< uint64_t >( 0 );
			int fcn    = int( sql.get_column_value< int64_t >( 1 ) );
            (void)fcn;
            
            if ( blob.open( dbf_.db(), "main", "AcquiredData", "data", rowid, adfs::readonly ) ) {
                xdata.resize( blob.size() );
                if ( blob.size() )
                    blob.read( reinterpret_cast< int8_t *>( xdata.data() ), blob.size() );
            }
            if ( blob.open( dbf_.db(), "main", "AcquiredData", "meta", rowid, adfs::readonly ) ) {
                xmeta.resize( blob.size() );
                if ( blob.size() )
                    blob.read( reinterpret_cast< int8_t *>( xmeta.data() ), blob.size() );
            }
            size_t idData = 0;
            return interpreter.translate( ms, xdata.data(), xdata.size()
                                          , xmeta.data(), xmeta.size(), spectrometer, idData++, traceId.c_str() );
        }
    }
    return adcontrols::translate_error;
}

bool
rawdata::hasProcessedSpectrum( int, int ) const
{
    return std::find_if( conf_.begin(), conf_.end()
                         , [](const adutils::AcquiredConf::data& d){ return d.trace_id == L"MS.CENTROID"; }) 
        != conf_.end();
}

uint32_t
rawdata::findObjId( const std::wstring& traceId ) const
{
    auto it =
        std::find_if( conf_.begin(), conf_.end(), [=](const adutils::AcquiredConf::data& d ){ return d.trace_id == traceId; });
    if ( it != conf_.end() )
        return uint32_t(it->objid);
    return 0;
}

bool
rawdata::getRaw( uint64_t objid, uint64_t npos, uint64_t& fcn, std::vector< char >& xdata, std::vector< char >& xmeta ) const
{
    adfs::stmt sql( dbf_.db() );
	
	xdata.clear();
	xmeta.clear();

    if ( sql.prepare( "SELECT rowid, fcn FROM AcquiredData WHERE oid = :oid AND npos = :npos" ) ) {

        sql.bind( 1 ) = uint64_t(objid);
        sql.bind( 2 ) = uint64_t(npos);

        adfs::blob blob;

        if ( sql.step() == adfs::sqlite_row ) {
            uint64_t rowid = sql.get_column_value< int64_t >( 0 );
            fcn            = sql.get_column_value< int64_t >( 1 );
            
            if ( blob.open( dbf_.db(), "main", "AcquiredData", "data", rowid, adfs::readonly ) ) {
                xdata.resize( blob.size() );
                if ( blob.size() )
                    blob.read( reinterpret_cast< int8_t *>( xdata.data() ), blob.size() );
            }
            if ( blob.open( dbf_.db(), "main", "AcquiredData", "meta", rowid, adfs::readonly ) ) {
                xmeta.resize( blob.size() );
                if ( blob.size() )
                    blob.read( reinterpret_cast< int8_t *>( xmeta.data() ), blob.size() );
            }
			return true;
        }
    }
    return adcontrols::translate_error;
}

