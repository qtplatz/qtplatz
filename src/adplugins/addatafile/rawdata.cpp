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
#include <memory>
#include <cstdint>

using namespace addatafile;

rawdata::~rawdata()
{
}

rawdata::rawdata( adfs::filesystem& dbf ) : dbf_( dbf )
                                          , npos0_( 0 )
{
}

bool
rawdata::loadAcquiredConf()
{
    conf_.clear();

    adfs::stmt sql( dbf_.db() );

    if ( sql.prepare( "SELECT objid, pobjid, dataInterpreterClsid, trace_method, spectrometer,\
 trace_id, trace_display_name, axis_x_label, axis_y_label, axis_x_decimals, axis_y_decimals FROM AcquiredConf" ) ) {

        while ( sql.step() == adfs::sqlite_row ) {
            
			AcquiredConf conf;
            
            conf.objid  = boost::get<int64_t>( sql.column_value( 0 ) );
            conf.pobjid = boost::get<int64_t>( sql.column_value( 1 ) );
            conf.dataInterpreterClsid = boost::get<std::wstring>( sql.column_value( 2 ) );
            conf.trace_method = boost::get<std::int64_t>( sql.column_value( 3 ) );
            conf.spectrometer = boost::get<std::int64_t>( sql.column_value( 4 ) );
            conf.trace_id = boost::get<std::wstring>( sql.column_value( 5 ) );
            conf.trace_display_name = boost::get<std::wstring>( sql.column_value( 6 ) );
            conf.axis_x_label = boost::get<std::wstring>( sql.column_value( 7 ) );
            conf.axis_y_label = boost::get<std::wstring>( sql.column_value( 8 ) );
            conf.axis_x_decimals = boost::get<std::int64_t>( sql.column_value( 9 ) );
            conf.axis_y_decimals = boost::get<std::int64_t>( sql.column_value( 10 ) );

            conf_.push_back( conf );
        }
    }

    for ( const auto& conf: conf_ ) {
        if ( conf.trace_method == signalobserver::eTRACE_TRACE // timed trace := chromatogram
             && conf.trace_id == L"MS.TIC" ) {

            adcontrols::TraceAccessor accessor;
            if ( fetchTraces( conf.objid, conf.dataInterpreterClsid, accessor ) ) {
                for ( size_t fcn = 0; fcn < accessor.nfcn(); ++fcn ) {
                    std::shared_ptr< adcontrols::Chromatogram > cptr( new adcontrols::Chromatogram() );
                    cptr->addDescription( adcontrols::Description( L"create",  conf.trace_display_name ) );
                    accessor.copy_to( *cptr, fcn );
                    tic_.push_back( cptr );
                }
            }
        }
    }
    
    return true;
}

size_t
rawdata::getSpectrumCount( int fcn ) const
{
    adfs::stmt sql( dbf_.db() );
    if ( sql.prepare( "SELECT count(rowid) FROM AcquiredData WHERE oid = :oid AND fcn = :fcn" ) ) {
        sql.bind( 1 ) = fcn;
        if ( sql.step() == adfs::sqlite_row )
            return static_cast< size_t >( boost::get<boost::int64_t>( sql.column_value( 0 ) ) );
    }
    return 0;
}

bool
rawdata::getSpectrum( int fcn, int idx, adcontrols::MassSpectrum& ms ) const
{
    (void)fcn;

	auto it = std::find_if( conf_.begin(), conf_.end(), []( const AcquiredConf& c ){
            return c.trace_method == signalobserver::eTRACE_SPECTRA && c.trace_id == L"MS.PROFILE";
        });

	if ( it != conf_.end() ) {
        uint64_t npos = npos0_ + idx;

        adfs::stmt sql( dbf_.db() );
        if ( sql.prepare( "SELECT max(npos) FROM AcquiredData WHERE oid = :oid AND fcn = 0 AND npos <= :npos" ) ) {
            sql.bind( 1 ) = it->objid;
            sql.bind( 2 ) = npos;
            if ( sql.step() == adfs::sqlite_row )
                npos = boost::get< boost::int64_t >(sql.column_value( 0 ));
        }
		return fetchSpectrum( it->objid, it->dataInterpreterClsid, npos, ms );
    }

    return false;
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
rawdata::posFromTime( double minute ) const
{
	if ( ! tic_.empty() ) {
		const adportable::array_wrapper< const double > times( tic_[0]->getTimeArray(), tic_[0]->size() );
		auto it = std::lower_bound( times.begin(), times.end(), tic_[0]->toSeconds( minute ) );
		
		return std::distance( times.begin(), it );
	}
	return 0;
}

bool
rawdata::getChromatograms( int fcn
                           , const std::vector< std::pair<double, double> >&
                           , std::vector< adcontrols::Chromatogram >&
                           , std::function< bool (long curr, long total ) > progress
                           , int begPos
                           , int endPos ) const
{
    (void)fcn;
    (void)progress;
    (void)begPos;
    (void)endPos;

	return false;
}

bool
rawdata::readCalibration( uint32_t objid, std::wstring& dataClass, std::vector< char >& device, uint64_t& rev ) const
{
    adfs::stmt sql( dbf_.db() );

    if ( sql.prepare( "SELECT rowid,revision,dataClass FROM Calibration WHERE objid = :objid ORDER BY revision desc" ) ) {
        
        sql.bind( 1 ) = objid;
        
        if ( sql.step() == adfs::sqlite_row ) {

            uint64_t rowid = boost::get<int64_t>( sql.column_value( 0 ) );
            rev = boost::get<int64_t>( sql.column_value( 1 ) );
			dataClass = boost::get< std::wstring >( sql.column_value( 2 ) );

            adfs::blob blob;
            if ( blob.open( dbf_.db(), "main", "Calibration", "data", rowid, adfs::readonly ) ) {
                if ( blob.size() ) {
                    return blob.read( reinterpret_cast< int8_t *>( device.data() ), device.size() );
                }
            }
        }
        return false;
    }
    return false;
}

bool
rawdata::getCalibration( int fcn, adcontrols::MSCalibrateResult& calibResult, adcontrols::MassSpectrum& calibSpectrum ) const
{
    (void)fcn;
    using adportable::serializer;
    using adcontrols::MSCalibrateResult;
    using adcontrols::MassSpectrum;

    uint64_t rev = 0;
    bool success = false;

    adfs::stmt sql( dbf_.db() );

    if ( sql.prepare( "SELECT rowid,revision FROM Calibration WHERE dataClass = :dataClass ORDER BY revision desc" ) ) {
        sql.bind( 1 ) = std::wstring( adcontrols::MSCalibrateResult::dataClass() );
        if ( sql.step() == adfs::sqlite_row ) {
            
            uint64_t rowid = boost::get< int64_t >( sql.column_value( 0 ) );
            rev = boost::get< int64_t >( sql.column_value( 1 ) );
            adfs::blob blob;
            if ( blob.open( dbf_.db(), "main", "Calibration", "data", rowid, adfs::readonly ) ) {
                std::vector< char > device( blob.size() );
                blob.read( reinterpret_cast< int8_t *>( device.data() ), device.size() );
                success = serializer< MSCalibrateResult >::deserialize( calibResult
                                                                        , reinterpret_cast<const char *>( device.data() )
                                                                        , device.size() );
            }
        }
    }
    if ( success ) {
        if ( sql.prepare( "SELECT rowid FROM Calibration WHERE dataClass = :dataClass AND revision = :rev" ) ) {
            sql.bind( 1 ) = std::wstring( adcontrols::MassSpectrum::dataClass() );
            sql.bind( 2 ) = rev;
            if ( sql.step() == adfs::sqlite_row ) {
                uint64_t rowid = boost::get< int64_t >( sql.column_value( 0 ) );
                adfs::blob blob;
                if ( blob.open( dbf_.db(), "main", "Calibration", "data", rowid, adfs::readonly ) ) {
                    std::vector< char > device( blob.size() );
                    blob.read( reinterpret_cast< int8_t *>( device.data() ), device.size() );
                    success = serializer< MassSpectrum >::deserialize( calibSpectrum
                                                                       , reinterpret_cast<const char *>( device.data() )
                                                                       , device.size() );
                }
            }
        }
    }
	return success;
}


// private
bool
rawdata::fetchTraces( int64_t objid, const std::wstring& dataInterpreterClsid, adcontrols::TraceAccessor& accessor )
{
    const adcontrols::MassSpectrometer& spectrometer = adcontrols::MassSpectrometer::get( dataInterpreterClsid );
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
            uint64_t rowid = boost::get< boost::int64_t >( sql.column_value( 0 ) );
            uint64_t npos = boost::get< boost::int64_t >( sql.column_value( 1 ) );
            uint32_t events = static_cast< uint32_t >( boost::get< boost::int64_t >( sql.column_value( 2 ) ) );
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
bool
rawdata::fetchSpectrum( int64_t objid
                        , const std::wstring& dataInterpreterClsid
                        , uint64_t npos, adcontrols::MassSpectrum& ms ) const
{
    const adcontrols::MassSpectrometer& spectrometer = adcontrols::MassSpectrometer::get( dataInterpreterClsid );
    const adcontrols::DataInterpreter& interpreter = spectrometer.getDataInterpreter();

    adfs::stmt sql( dbf_.db() );

    if ( sql.prepare( "SELECT rowid, fcn FROM AcquiredData WHERE oid = :oid AND npos = :npos" ) ) {

        sql.bind( 1 ) = objid;
        sql.bind( 2 ) = npos;

        adfs::blob blob;
        std::vector< char > xdata;
        std::vector< char > xmeta;

        if ( sql.step() == adfs::sqlite_row ) {
            uint64_t rowid  = boost::get< boost::int64_t >( sql.column_value( 0 ) );
            uint64_t fcn    = boost::get< boost::int64_t >( sql.column_value( 1 ) );
            
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
            interpreter.translate( ms, xdata.data(), xdata.size(), xmeta.data(), xmeta.size(), spectrometer, idData++ );
            return true;
        }
    }

    return false;
}

rawdata::AcquiredConf::AcquiredConf() : objid( 0 )
                                      , pobjid( 0 )
                                      , trace_method( 0 )
                                      , spectrometer( 0 )
                                      , axis_x_decimals( 0 )
                                      , axis_y_decimals( 0 )
{
}

rawdata::AcquiredConf::AcquiredConf( const AcquiredConf& t ) : objid( t.objid )
                                                             , pobjid( t.pobjid )
                                                             , trace_method( t.trace_method )
                                                             , spectrometer( t.spectrometer )
                                                             , dataInterpreterClsid( t.dataInterpreterClsid )
                                                             , trace_id( t.trace_id )
                                                             , trace_display_name( t.trace_display_name )
                                                             , axis_x_label( t.axis_x_label )
                                                             , axis_y_label( t.axis_y_label )
                                                             , axis_x_decimals( t.axis_x_decimals )
                                                             , axis_y_decimals( t.axis_y_decimals )
{
}

