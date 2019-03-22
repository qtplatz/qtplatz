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
#include <adacquire/signalobserver.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/lockmass.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/msfractuation.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/traceaccessor.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/debug.hpp>
#include <adportable/utf.hpp>
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
                                                   , fcnCount_( 0 )
                                                   , npos0_( 0 )
                                                   , configLoaded_( false )
{
}

adfs::sqlite *
rawdata::db() const
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
            if ( auto reader = adcontrols::DataReader::make_reader( conf.trace_id.c_str() ) ) {
                if ( reader->initialize( dbf_, conf.objid, conf.objtext ) ) {
                    reader->setDescription( adacquire::SignalObserver::eTRACE_METHOD( conf.trace_method )
                                            , conf.trace_id
                                            , adportable::utf::to_utf8( conf.trace_display_name )
                                            , adportable::utf::to_utf8( conf.axis_label_x )
                                            , adportable::utf::to_utf8( conf.axis_label_y )
                                            , conf.axis_decimals_x
                                            , conf.axis_decimals_y );
                    readers_.emplace_back( reader, int( reader->fcnCount() ) );
                }
            } else {
                undefined_data_readers_.emplace_back( conf.objtext, conf.objid );
                ADERROR() << "# reader for '" << conf.trace_id << "'" << " not implemented.";
            }
        }
        fcnCount_ = 0;
        for ( auto reader : readers_ )
            fcnCount_ += reader.second; // fcnCount
        return true;
    }
    return false;
}

void
rawdata::loadMSFractuation()
{
    using adcontrols::lockmass::mslock;
    using adcontrols::lockmass::reference;

    auto fractuation = adcontrols::MSFractuation::create();

    {
        adfs::stmt sql( dbf_.db() );
        sql.prepare( "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='MSLock'" );
        if ( sql.step() == adfs::sqlite_row ) {
            if ( sql.get_column_value< int64_t >( 0 ) == 0 )
                return;
        }
    }

    adfs::stmt sql( dbf_.db() );
    sql.prepare( "SELECT DISTINCT rowid FROM MSLock ORDER BY rowid" );

    while ( sql.step() == adfs::sqlite_row ) {

        int64_t rowid = sql.get_column_value< int64_t >(0);

        adfs::stmt sql2( dbf_.db() );
        sql2.prepare( "SELECT exactMass,matchedMass FROM MSLock where rowid = ?" );
        sql2.bind( 1 ) = rowid;

        mslock mslock;

        while ( sql2.step() == adfs::sqlite_row ) {
            auto exactMass = sql2.get_column_value< double >( 0 );
            auto matchedMass = sql2.get_column_value< double >( 1 );
            mslock << reference( "", exactMass, matchedMass, 0 );
        }

        if ( mslock.fit() )
            fractuation->insert( rowid, mslock );
    }

    for ( auto reader : readers_ ) {
        if ( auto spectrometer = reader.first->massSpectrometer() )
            spectrometer->setMSFractuation( fractuation.get() );
    }

}


size_t
rawdata::dataReaderCount() const
{
    return readers_.size();
}

const adcontrols::DataReader *
rawdata::dataReader( size_t idx ) const
{
    if ( idx < readers_.size() )
        return readers_ [ idx ].first.get();
    return nullptr;
}

const adcontrols::DataReader *
rawdata::dataReader( const boost::uuids::uuid& uuid ) const
{
    typedef std::pair < std::shared_ptr< adcontrols::DataReader >, int> value_type;

    auto it = std::find_if( readers_.begin(), readers_.end(), [&] ( const value_type& a ) { return a.first->objuuid() == uuid; } );
    if ( it != readers_.end() )
        return it->first.get();
    return nullptr;
}

std::vector < std::shared_ptr< const adcontrols::DataReader > >
rawdata::dataReaders( bool allPossible ) const
{
    std::vector < std::shared_ptr< const adcontrols::DataReader > > v;
    v.reserve( readers_.size() );

    if ( allPossible ) {
        for ( auto& reader : readers_ )
            v.push_back( reader.first );
    } else {
        for ( auto& reader : readers_ )
            if ( reader.first->fcnCount() > 0 || reader.first->trace_method() == adacquire::SignalObserver::eTRACE_TRACE )
                v.emplace_back( reader.first );
    }

    return v;
}

bool
rawdata::applyCalibration( const std::wstring& dataInterpreterClsid, const adcontrols::MSCalibrateResult& calibResult )
{
    return false;
}

void
rawdata::loadCalibrations()
{
}

std::shared_ptr< adcontrols::MassSpectrometer >
rawdata::getSpectrometer( uint64_t objid, const std::wstring& dataInterpreterClsid ) const
{
	assert(0);
	return nullptr;
    //return const_cast<rawdata *>(this)->getSpectrometer( objid, dataInterpreterClsid );
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
    assert( 0 );
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
    //--------------
    // When this method was desigened at 2010, the number 'pos' that is trigger id was unique in the datafile.
    // But 'pos' no longer unique due to simultaneous 'counting' and 'soft-averaged waveforms' data streams.
    // The rawid from datafile is only be unique.

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

    idx = int( std::count_if( fcnIdx_.begin(), index, [] ( value_type& a ){ return a.second == 0; } ) );

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
        auto it = std::find_if( fcnIdx_.rbegin(), fcnIdx_.rend(), [] ( value_type& a ){ return a.second == 0; } );
        if ( it != fcnIdx_.rend() ) {
            // find next fcn=0
            it = std::find_if( ++it, fcnIdx_.rend(), [] ( value_type& a ){ return a.second == 0; } );
            if ( it != fcnIdx_.rend() )
                return it->first - npos0_;
        }
        return size_t(-1);
    }

    if ( idx >= 0 && fcn < 0 ) {  // find first "replicates-alinged" scan pos (idx means tic[fcn][idx])
        size_t count = 0;
        for ( auto it = fcnIdx_.begin(); it != fcnIdx_.end(); ++it ) {
            if ( it->second == 0 && count++ == size_t(idx) )
                return it->first - npos0_;  // 1st "replicate = 0" data for idx'th fcn = 0
        }
        return size_t( -1 );
    }

    if ( idx < 0 && fcn >= 0 ) { // find last data for specified protocol
        typedef decltype(*fcnIdx_.rbegin()) value_type;
        auto it = std::find_if( fcnIdx_.rbegin(), fcnIdx_.rend(), [fcn] ( value_type& a ){ return a.second == fcn; } );
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
        auto idx = std::count_if( fcnVec_.begin(), fcnVec_.begin() + pos, [=] ( iterator& v ){return std::get<1>( v ) == fcn; } );

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
        if ( auto tpos = reader.first->findPos( seconds ) )
            return tpos->time_since_inject();
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
    result.clear();
    return false;
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
    // this method no longer supported for v3
    assert( 0 );
    return adcontrols::no_interpreter;
}

bool
rawdata::hasProcessedSpectrum( int, int ) const
{
    // this method no longer supported for v3
    assert( 0 );
    return false;
}

uint32_t
rawdata::findObjId( const std::wstring& traceId ) const
{
    // this method no longer supported for v3
    assert( 0 );
    return false;
}

bool
rawdata::getRaw( uint64_t objid, uint64_t npos, uint64_t& fcn, std::vector< char >& xdata, std::vector< char >& xmeta ) const
{
    // this method no longer supported for v3
    assert( 0 );
    return false;
}

bool
rawdata::mslocker( adcontrols::lockmass::mslock &mslk, uint32_t objid ) const
{
    // this method no longer supported for v3
    assert( 0 );
    return false;
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
