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

#include "rawdata_v3.hpp"
#include <adicontroller/signalobserver.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/traceaccessor.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/debug.hpp>
#include <adplugin_manager/manager.hpp>
#include <adfs/adfs.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adlog/logger.hpp>
#include <adutils/mscalibio.hpp>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <algorithm>
#include <memory>
#include <cstdint>
#include <sstream>
#include <set>
#include <regex>

using namespace addatafile;
using namespace addatafile::v3;

rawdata::~rawdata()
{
}

rawdata::rawdata( adfs::filesystem& dbf
                  , adcontrols::datafile& parent ) : dbf_( dbf )
                                                   , parent_( parent )
                                                   , npos0_( 0 )
                                                   , configLoaded_( false )
                                                   , fcnCount_( 0 )
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

    if ( adutils::v3::AcquiredConf::fetch( dbf_.db(), conf_ ) && !conf_.empty() ) {

        for ( const auto& conf: conf_ ) {
            ADDEBUG() << conf.trace_method << ", " << conf.trace_id;
            if ( auto reader = adcontrols::DataReader::make_reader( conf.trace_id.c_str() ) ) {
                if ( reader->initialize( dbf_, conf.objid, conf.objtext ) ) {
                    readers_.push_back( std::make_pair( reader, int( reader->fcnCount() ) ) );
                }
            }
        }
        fcnCount_ = 0;
        for ( auto reader : readers_ )
            fcnCount_ += reader.second; // fcnCount
        return true;
    }
    
    return false;
}

bool
rawdata::applyCalibration( const std::wstring& dataInterpreterClsid, const adcontrols::MSCalibrateResult& calibResult )
{
#if 0
    boost::uuids::uuid objid;
    
    auto it = std::find_if( conf_.begin(), conf_.end(), [=](const adutils::AcquiredConf::data& c){
            return c.dataInterpreterClsid == dataInterpreterClsid;
        });

    if ( it != conf_.end() )
        objid = it->objid;
    else {
        if ( conf_.empty() ) 
            adutils::v3::AcquiredConf::create_table_v3( dbf_.db() );
        adutils::v3::AcquiredConf::data d;
		d.objid = objid;
        d.dataInterpreterClsid.resize( dataInterpreterClsid.size() );
        std::copy( dataInterpreterClsid.begin(), dataInterpreterClsid.end(), d.dataInterpreterClsid.begin() );
        if ( !adutils::v3::AcquiredConf::insert( dbf_.db(), objid, d ) )
            return false;
    }

	const std::wstring calibId = calibResult.calibration().calibId();
    std::string device;
    if ( adportable::binary::serialize<>()(calibResult, device) ) {
        adutils::mscalibio::writeCalibration( dbf_.db(), uint32_t( objid ), calibId.c_str(), calibResult.dataClass(), device.data(), device.size() );
        loadAcquiredConf();
        loadCalibrations();
        return true;
    }
#endif
    return false;
}

void
rawdata::loadCalibrations()
{
#if 0
    // using adportable::serializer;
    using adcontrols::MSCalibrateResult;

    std::for_each( conf_.begin(), conf_.end(), [&]( const adutils::AcquiredConf::data& conf ){
            std::vector< char > device;
            int64_t rev;
            if ( adutils::mscalibio::readCalibration( dbf_.db(), uint32_t(conf.objid), MSCalibrateResult::dataClass(), device, rev ) ) {

                auto calibResult = std::make_shared< MSCalibrateResult >();
                
                boost::iostreams::basic_array_source< char > source( device.data(), device.size() );
                boost::iostreams::stream< boost::iostreams::basic_array_source< char > > strm( source );

                if ( MSCalibrateResult::restore( strm, *calibResult ) ) {
                    calibResults_[ conf.objid ] = calibResult;
                    if ( auto spectrometer = getSpectrometer( conf.objid, conf.dataInterpreterClsid.c_str() ) )
                        spectrometer->setCalibration( calibResult->mode(), *calibResult );
                }
            }
        });
#endif
}

std::shared_ptr< adcontrols::MassSpectrometer >
rawdata::getSpectrometer( uint64_t objid, const std::wstring& dataInterpreterClsid )
{
#if 0
    auto it = spectrometers_.find( objid );
    if ( it == spectrometers_.end() ) {
		if ( auto ptr = adcontrols::MassSpectrometer::create( dataInterpreterClsid.c_str(), &parent_ ) ) {
            spectrometers_[ objid ] = ptr;
			it = spectrometers_.find( objid );
		} else {
            return 0;
			//static adcontrols::MassSpectrometer x;
			//return x;
		}
    }
    return it->second;
#endif
    return 0;
}

std::shared_ptr< adcontrols::MassSpectrometer >
rawdata::getSpectrometer( uint64_t objid, const std::wstring& dataInterpreterClsid ) const
{
    return const_cast<rawdata *>(this)->getSpectrometer( objid, dataInterpreterClsid );
}

size_t
rawdata::getSpectrumCount( int fcn ) const
{
    int tfcn(0);
    if ( auto reader = findDataReader( fcn, tfcn ) ) {
        if ( auto tic = reader->TIC( tfcn ) )
            return tic->size();
    }
    return 0;
}

bool
rawdata::getSpectrum( int fcn, size_t pos, adcontrols::MassSpectrum& ms, uint32_t objid ) const
{
    boost::uuids::uuid objuuid = { 0 };
    std::vector< boost::uuids::uuid > uuids;

    adfs::stmt sql( dbf_.db() );
    if ( sql.prepare( "SELECT objuuid from AcquiredData WHERE AcquiredData.npos = ?" ) ) {
        sql.bind( 1 ) = pos;
        while ( sql.step() == adfs::sqlite_row ) 
            uuids.push_back( sql.get_column_value< boost::uuids::uuid >( 0 ) );
    }

    return true;
}

bool
rawdata::index( size_t pos, int& idx, int& fcn, int& rep, double * time ) const
{
    // idx   pos   fcn   rep  (assume replicates = 3, protocols(fcn) = 3
    // 0     1001   0     0
    //       1002   0     1
    //       1003   0     2
    //       1004   1     0 << change fcn
    //       1005   1     1
    //       1006   1     2
    //       1007   2     0 << chanee fcn
    //       1008   2     1
    //       1009   2     2
    // 1     1010   0     0 << change fcn, back to 0
    //       1011   0     1

    if ( fcnIdx_.size() == 1 ) { // no protocol sequence acquisition
        if ( pos >= fcnVec_.size() )
            return false;
        fcn = std::get<1>( fcnVec_[ pos ] );
        rep = 0;
        idx = int( pos );
        if ( time )
            *time = timeFromPos( pos );
        return true;
    }

    auto index = std::lower_bound( fcnIdx_.begin(), fcnIdx_.end(), pos + npos0_
                                       , [] ( const std::pair< size_t, int >& a, size_t npos ) { return a.first < npos; } );
    if ( index == fcnIdx_.end() )
        return false;

    while ( index != fcnIdx_.begin() && index->first > ( pos + npos0_ ) )
        --index;

    typedef decltype(*fcnIdx_.begin()) value_type;

    idx = int( std::count_if( fcnIdx_.begin(), index, [] ( const value_type& a ){ return a.second == 0; } ) );

    if ( pos < fcnVec_.size() ) {
        fcn = std::get<1>( fcnVec_[ pos ] );
        rep = std::get<2>( fcnVec_[ pos ] );
        if ( time )
            *time = timeFromPos( pos );
        return true;
    }
    return false;
}

size_t
rawdata::find_scan( int idx, int fcn ) const
{
    if ( fcnIdx_.empty() )
        return size_t(-1);

    if ( idx < 0 && fcn < 0 ) { // find last data can be read for a set of entire protocols
        typedef decltype(*fcnIdx_.rbegin()) value_type;
        auto it = std::find_if( fcnIdx_.rbegin(), fcnIdx_.rend(), [] ( const value_type& a ){ return a.second == 0; } );
        if ( it != fcnIdx_.rend() ) {
            // find next fcn=0
            it = std::find_if( ++it, fcnIdx_.rend(), [] ( const value_type& a ){ return a.second == 0; } );
            if ( it != fcnIdx_.rend() )
                return it->first - npos0_;
        }
        return size_t(-1);
    }

    if ( idx >= 0 && fcn < 0 ) {  // find first "replicates-alinged" scan pos (idx means tic[fcn][idx])
        typedef decltype(*fcnIdx_.rbegin()) value_type;
        size_t count = 0;
        for ( auto it = fcnIdx_.begin(); it != fcnIdx_.end(); ++it ) {
            if ( it->second == 0 && count++ == size_t(idx) )
                return it->first - npos0_;  // 1st "replicate = 0" data for idx'th fcn = 0
        }
        return size_t( -1 );
    }

    if ( idx < 0 && fcn >= 0 ) { // find last data for specified protocol
        typedef decltype(*fcnIdx_.rbegin()) value_type;
        auto it = std::find_if( fcnIdx_.rbegin(), fcnIdx_.rend(), [fcn] ( const value_type& a ){ return a.second == fcn; } );
        if ( it != fcnIdx_.rend() )
            return it->first - npos0_; // 1st spectrum of a set of acquisition replicates for specified protocol (fcn)
        return size_t(-1);
    }

    // TIC based scan#, this will return not "replicates" aligned data pos
    if ( size_t(fcn) < tic_.size() ) {
        double t = tic_[ fcn ]->getTimeArray()[ idx ];
        auto it = std::lower_bound( times_.begin(), times_.end(), t, [] ( const std::pair<double, int>& a, double t ){ return a.first < t; } );
        if ( it != times_.end() )
            return std::distance( times_.begin(), it );
    }

    return size_t(-1);
}

int
rawdata::make_index( size_t pos, int& fcn ) const
{
    fcn = 0;
    if ( pos < fcnVec_.size() ) {

        fcn = std::get<1>( fcnVec_[ pos ] );

        typedef decltype( *fcnVec_.begin() ) iterator;
        auto idx = std::count_if( fcnVec_.begin(), fcnVec_.begin() + pos, [=] ( const iterator& v ){return std::get<1>( v ) == fcn; } );

        return int( idx );
    }
    return 0;
}

bool
rawdata::getTIC( int fcn, adcontrols::Chromatogram& c ) const
{
    int tfcn( 0 );
    if ( auto reader = findDataReader( fcn, tfcn ) ) {
        if ( auto pchro = reader->TIC( tfcn ) ) {
            c = *pchro;
            return true;
        }
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
    return fcnCount_;
}

size_t
rawdata::posFromTime( double seconds ) const
{
    for ( auto& reader : readers_ ) {

        auto tpos = reader.first->findPos( seconds );
        /*
        if ( tpos != reader.first->end() ) {
            ADDEBUG() << tpos->pos();
            return tpos->time_since_inject();
        }
        */
    }
	return 0;
}

double
rawdata::timeFromPos( size_t pos ) const
{
    if ( !times_.empty() && pos < times_.size() )
        return times_[ pos ].first;
	return 0;
}

bool
rawdata::getChromatograms( const std::vector< std::tuple<int, double, double> >& ranges
                           , std::vector< adcontrols::Chromatogram >& result
                           , std::function< bool (long curr, long total ) > progress
                           , int begPos
                           , int endPos ) const
{
    (void)begPos;
    (void)endPos;
    result.clear();
#if 0    
	auto it = std::find_if( conf_.begin(), conf_.end(), []( const adutils::AcquiredConf::data& c ){
        return c.trace_method == signalobserver::eTRACE_SPECTRA && c.trace_id == L"MS.PROFILE";  } );
    
	if ( it == conf_.end() )
        return false;

    auto spectrometer = getSpectrometer( it->objid, it->dataInterpreterClsid.c_str() );
    if ( !spectrometer )
        return false;

    const adcontrols::DataInterpreter& interpreter = spectrometer->getDataInterpreter();
    
    typedef std::tuple< int, double, double, std::vector< adcontrols::Chromatogram >::size_type > mass_window_t;
    std::vector< mass_window_t > masses;

    for ( auto& range: ranges ) {

        result.push_back( adcontrols::Chromatogram() );
        auto idChro = result.size() - 1;
        
        int fcn = std::get<0>( range );
        
        if ( std::get<2>( range ) <= 1.0 ) {
            double mass = std::get<1>( range );
            double width = std::get<2>( range );
            result.back().addDescription( adcontrols::description( L"Create"
                                                                  , ( boost::wformat( L"m/z %.4lf (W:%.4fmDa) %d" ) % mass % ( width * 1000 ) % fcn ).str() ) );
            masses.push_back( std::make_tuple( fcn, mass - width / 2, mass + width / 2, idChro ) );
        } else {
            double lMass = std::get<1>( range );
            double uMass = std::get<2>( range );
            result.back().addDescription( adcontrols::description( L"Create"
                                                            , ( boost::wformat( L"m/z (%.4lf - %.4lf) %d" ) % lMass % uMass % fcn ).str() ) );
            masses.push_back( std::make_tuple( fcn, lMass, uMass, idChro ) );
        }
    }
    
    std::sort( masses.begin(), masses.end(), []( const mass_window_t& a, const mass_window_t& b ){ return std::get<1>(a) < std::get<1>(b); } );
    
    int nProgress = 0;
    adfs::stmt sql( dbf_.db() );

    uint64_t pos = npos0_;

    for ( int i = 0; i < tic_[0]->size(); ++i ) {

        progress( nProgress++, long( tic_[ 0 ]->size() ) );
        
        adcontrols::MassSpectrum _ms;
        adcontrols::translate_state state;

        // read all protocols
        while ( ( state = fetchSpectrum( it->objid, it->dataInterpreterClsid, pos++, _ms, it->trace_id ) ) == adcontrols::translate_indeterminate )
            ;

        if ( state == adcontrols::translate_complete ) {

            adcontrols::segment_wrapper<> segments( _ms );
            for( auto& fms: segments ) {

                double base(0), rms(0);
                double tic = adportable::spectrum_processor::tic( uint32_t(fms.size()), fms.getIntensityArray(), base, rms );
                double time = fms.getMSProperty().timeSinceInjection();
                if ( time > 4000 ) // workaround for negative time at the begining of time event function delay
                    time = 0;
                
                for ( auto& t: masses ) {
                    
                    double lMass = std::get<1>( t );
                    double uMass = std::get<2>( t );

                    if ( fms.getMass( 0 ) < lMass && uMass < fms.getMass( fms.size() - 1 ) ) {
                        auto idChro = std::get<3>( t );

                        adportable::spectrum_processor::areaFraction fraction;
                        adportable::spectrum_processor::getFraction( fraction, fms.getMassArray(), fms.size(), lMass, uMass );

                        double d = adportable::spectrum_processor::area( fraction, base, fms.getIntensityArray(), fms.size() );

                        result[ idChro ] << std::make_pair( time, d );

                        ADDEBUG() << " tic=" << tic << ", mass=(" << lMass << ", " << uMass
                                  << "), frac=(" << fraction.lPos << ", " << fraction.uPos << ", " << fraction.lFrac << ", " << fraction.uFrac
                                  << "), d="  << d << ", time=" << fms.getMSProperty().timeSinceInjection();
                    }
                }

            }
        } else if ( state == adcontrols::no_more_data ) {
            return true;
        }
    }
#endif
    return true;
}

// private
bool
rawdata::fetchTraces( int64_t objid, const adcontrols::DataInterpreter& interpreter, adcontrols::TraceAccessor& accessor )
{

    adfs::stmt sql( dbf_.db() );
    
    if ( sql.prepare( "SELECT rowid, npos, events, fcn FROM AcquiredData WHERE oid = :oid ORDER BY npos" ) ) {

        sql.bind( 1 ) = objid;
        adfs::blob blob;
        std::vector< char > xdata;
        std::vector< char > xmeta;
        size_t nrecord = 0;

        while ( sql.step() == adfs::sqlite_row ) {
            ++nrecord;
            uint64_t rowid = sql.get_column_value< int64_t >( 0 );
            uint64_t npos = sql.get_column_value< int64_t >( 1 );
            uint32_t events = static_cast<uint32_t>(sql.get_column_value< int64_t >( 2 ));
            if ( npos0_ == 0 )
                npos0_ = npos;

            if ( blob.open( dbf_.db(), "main", "AcquiredData", "data", rowid, adfs::readonly ) ) {
                xdata.resize( blob.size() );
                if ( blob.size() )
                    blob.read( reinterpret_cast<int8_t *>(xdata.data()), blob.size() );
            }

            if ( blob.open( dbf_.db(), "main", "AcquiredData", "meta", rowid, adfs::readonly ) ) {
                xmeta.resize( blob.size() );
                if ( blob.size() )
                    blob.read( reinterpret_cast<int8_t *>(xmeta.data()), blob.size() );
            }

            interpreter.translate( accessor, xdata.data(), xdata.size(), xmeta.data(), xmeta.size()
                                   , static_cast<uint32_t>(events) );
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
    if ( auto spectrometer = getSpectrometer( objid, dataInterpreterClsid ) ) {

        const adcontrols::DataInterpreter& interpreter = spectrometer->getDataInterpreter();

        adfs::stmt sql( dbf_.db() );

        if ( sql.prepare( "SELECT fcn, data, meta FROM AcquiredData WHERE oid = :oid AND npos = :npos" ) ) {
        
            sql.bind( 1 ) = objid;
            sql.bind( 2 ) = npos;

            if ( sql.step() == adfs::sqlite_row ) {
                int fcn = int( sql.get_column_value< int64_t >( 0 ) );
                (void)fcn;
                adfs::blob xdata = sql.get_column_value< adfs::blob >( 1 );
                adfs::blob xmeta = sql.get_column_value< adfs::blob >( 2 );

                size_t idData = 0;
                return interpreter.translate( ms, reinterpret_cast< const char *>(xdata.data()), xdata.size()
                                              , reinterpret_cast<const char *>(xmeta.data()), xmeta.size(), *spectrometer, idData++, traceId.c_str() );

            }
        }
        return adcontrols::no_more_data;
    }
    return adcontrols::no_interpreter;
}

bool
rawdata::hasProcessedSpectrum( int, int ) const
{
#if 0
    return std::find_if( conf_.begin(), conf_.end()
                         , [](const adutils::AcquiredConf::data& d){ return d.trace_id == L"MS.CENTROID"; }) 
        != conf_.end();
#endif
    return false;
}

uint32_t
rawdata::findObjId( const std::wstring& traceId ) const
{
#if 0
    auto it =
        std::find_if( conf_.begin(), conf_.end(), [=](const adutils::AcquiredConf::data& d ){ return d.trace_id == traceId; });
    if ( it != conf_.end() )
        return uint32_t(it->objid);
#endif
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

bool
rawdata::mslocker( adcontrols::lockmass& mslk, uint32_t objid ) const
{
    return false;
#if 0
    if ( objid == 0 ) {
        auto it = std::find_if( conf_.begin(), conf_.end(), [=]( const adutils::AcquiredConf::data& c ){
                return c.trace_method == signalobserver::eTRACE_SPECTRA && c.trace_id == L"MS.PROFILE";
            });
        if ( it == conf_.end() )
            return false;
        objid = uint32_t( it->objid );
    }

    auto it = spectrometers_.find( objid );
    if ( it != spectrometers_.end() ) {
        auto& interpreter = it->second->getDataInterpreter();
        if ( interpreter.has_lockmass() )
            return interpreter.lockmass( mslk );
    }
    return false;
#endif
}

std::shared_ptr< adcontrols::DataReader >
rawdata::findDataReader( int fcn, int& xfcn ) const
{
    auto it = readers_.begin();

    while ( it != readers_.end() ) {
        if ( fcn < it->second ) { // nfcn
            xfcn = fcn;
            return it->first;
        }
        fcn -= it->second;
        it++;
    }
    xfcn = 0;
    return nullptr;
}
