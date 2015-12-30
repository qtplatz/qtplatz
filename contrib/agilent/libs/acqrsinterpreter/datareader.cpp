/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "datareader.hpp"
#include "datareader_iterator.hpp"
#include "datainterpreter.hpp"
#include "datainterpreter_u5303a.hpp"
#include "datainterpreter_histogram.hpp"
#include "datainterpreter_timecount.hpp"
#include "datainterpreter_softavgr.hpp"
#include <acqrscontrols/u5303a/threshold_result.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/waveform.hpp>
#include <adportable/debug.hpp>
#include <adportable/utf.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <boost/format.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#if defined _MSC_VER && _MSC_VER <= 1800
# include <compiler/make_unique.hpp>
#endif

namespace acqrsinterpreter {

    template< typename Interpreter > struct TID {
        static const std::string value;
        typedef Interpreter type;
    };

    template<> const std::string TID<u5303a::DataInterpreter >::value = "1.u5303a.ms-cheminfo.com";
    template<> const std::string TID<timecount::DataInterpreter >::value = "timecount.1.u5303a.ms-cheminfo.com";
    template<> const std::string TID<histogram::DataInterpreter >::value = "histogram.timecount.1.u5303a.ms-cheminfo.com";
    template<> const std::string TID<softavgr::DataInterpreter>::value = "tdcdoc.waveform.1.u5303a.ms-cheminfo.com";

    typedef boost::mpl::vector<
        TID<u5303a::DataInterpreter >
        , TID<timecount::DataInterpreter>
        , TID<histogram::DataInterpreter>
        , TID<softavgr::DataInterpreter> > interpreter_types;

    template< typename T > struct wrap {};

    struct make_list {
        std::vector< std::string >& list;
        make_list( std::vector< std::string >& _list ) : list(_list ){}
        template < typename T > void operator () ( wrap<T> ) const {
            list.push_back( T::value );
        }
    };
        
    struct lookup_and_create {
        const char * id;
        std::unique_ptr< adcontrols::DataInterpreter >& interpreter;
        lookup_and_create( const char * _id, std::unique_ptr< adcontrols::DataInterpreter >& t ) : id( _id ), interpreter( t ) {}
        template < typename T > void operator () ( wrap<T> ) const {
            if ( id == T::value ) {
                interpreter = std::unique_ptr< typename T::type >( new typename T::type() );
            }
        }
    };

    struct total_ion_count : public boost::static_visitor< double > {
        template< typename T >
        double operator()( T& ptr ) const {
            return ptr->accumulate( 0.0, 0.0 ); // tic
        }
    };

    template<> double total_ion_count::operator()( std::shared_ptr< acqrscontrols::u5303a::threshold_result >& ptr ) const
    {
        return ptr->indecies().size();
    }

    struct make_title : public boost::static_visitor < std::wstring > {
        std::wstring operator()( std::shared_ptr< acqrscontrols::u5303a::threshold_result> & ) const {
            return ( boost::wformat( L"TDC" ) ).str();
        }
        std::wstring operator()( std::shared_ptr< adcontrols::TimeDigitalHistogram> & ) const {
            return ( boost::wformat( L"Histogram" ) ).str();
        }
        std::wstring operator()( std::shared_ptr< acqrscontrols::u5303a::waveform >& ) const {
            return ( boost::wformat( L"Averaged" ) ).str();
        }
    };

}

using namespace acqrsinterpreter;

DataReader::~DataReader()
{
}

DataReader::DataReader( const char * traceid ) : adcontrols::DataReader( traceid )
{
    // traceid determines type of trace, a.k.a. type of mass-spectormeter, multi-dimentional chromatogram etc.
    // Though traceid does not indiecate trace object (in case two UV-ditectors on the system, traceid does not tell which one)

    boost::mpl::for_each< interpreter_types, wrap< boost::mpl::placeholders::_1> >( lookup_and_create( traceid, interpreter_ ) );
}

// static
std::vector< std::string >
DataReader::traceid_list()
{
    std::vector< std::string > list;
    boost::mpl::for_each< interpreter_types, wrap< boost::mpl::placeholders::_1> >( make_list( list ) );
    return list;
}

bool
DataReader::initialize( adfs::filesystem& dbf, const boost::uuids::uuid& objid, const std::string& objtext )
{
    if ( interpreter_ ) {
        ADDEBUG() << "initialize data for: " << objtext;
        objid_ = objid; // objid tells channel/module id
        objtext_ = objtext; // for debugging convension
        db_ = dbf._ptr();
        return true;
    }
    ADDEBUG() << "initialize failed for: " << objtext;
    return false;
}

void
DataReader::finalize()
{
}

size_t
DataReader::fcnCount() const
{
    // skip timecount data -- too large to handle in the dataproc
    if ( auto i = interpreter_->_narrow< timecount::DataInterpreter >() ) {
        ADDEBUG() << "Timecount dataInterpreter found -- skip data.";
        return 0;
    }

    if ( auto db = db_.lock() ) {

        adfs::stmt sql( *db );
        sql.prepare( "SELECT COUNT( DISTINCT fcn ) FROM AcquiredData WHERE objuuid = ?" );
        sql.bind( 1 ) = objid_;
        
        size_t fcnCount( 0 );
        while ( sql.step() == adfs::sqlite_row )
            fcnCount += sql.get_column_value< int64_t >( 0 );
        
        return fcnCount;
    }
    return 0;
}

adcontrols::DataReader::const_iterator
DataReader::begin() const
{
    //return adcontrols::DataReader_iterator( std::unique_ptr< DataReader_index >( new DataReader_index( *this ) ) );
    return adcontrols::DataReader_iterator( std::make_unique< DataReader_index >(*this) );
}

adcontrols::DataReader::const_iterator
DataReader::end() const
{
    //return adcontrols::DataReader_iterator<>( std::make_unique< adcontrols::DataReader_index >(*this) );
    return adcontrols::DataReader_iterator( std::unique_ptr< DataReader_index >( new DataReader_index( *this ) ) );
}

adcontrols::DataReader::const_iterator
DataReader::findPos( double seconds, bool closest, TimeSpec tspec ) const
{
    //return adcontrols::DataReader_iterator<>( std::make_unique< adcontrols::DataReader_index >(*this) );
    return adcontrols::DataReader_iterator( std::unique_ptr< DataReader_index >( new DataReader_index( *this ) ) );    
#if 0
    if ( indecies_.empty() )
        return std::make_pair(-1,0);

    if ( tspec == EpochTime ) {
        assert( 0 ); // tba
        return std::make_pair(-1,0);
    }

    if ( tspec == ElapsedTime ) {

        int64_t elapsed_time = int64_t( seconds * 1e9 + 0.5 ) + indecies_.front().elapsed_time;

        if ( indecies_.front().elapsed_time > elapsed_time )
            return std::make_pair( indecies_.front().pos, time_since_inject( indecies_.front().elapsed_time ) );

        if ( indecies_.back().elapsed_time < elapsed_time )
            return std::make_pair(indecies_.back().pos, time_since_inject( indecies_.back().elapsed_time ) );

        auto its = std::lower_bound( indecies_.begin(), indecies_.end(), elapsed_time, [] ( const index& a, int64_t b ) { return a.elapsed_time < b; } );
        auto ite = std::upper_bound( indecies_.begin(), indecies_.end(), elapsed_time, [] ( int64_t a, const index& b ) { return a < b.elapsed_time; } );

        auto it = std::min_element( its, ite, [elapsed_time] ( const index& a, const index& b ) { return std::abs( elapsed_time - a.elapsed_time ) < std::abs( elapsed_time - b.elapsed_time ); } );

        return std::make_pair( it->pos, time_since_inject( it->elapsed_time ) );
    }

    return std::make_pair(-1,0);
#endif
}

double
DataReader::findTime( int64_t pos, IndexSpec ispec, bool exactMatch ) const 
{
    assert( ispec == TriggerNumber );

    if ( indecies_.empty() )
        return -1;
    auto it = std::lower_bound( indecies_.begin(), indecies_.end(), pos, [] ( const index& a, int64_t b ) { return a.pos < b; } );
    if ( it != indecies_.end() )
        return double( it->elapsed_time ) * 1.0e-9;

    return -1.0;
}

std::shared_ptr< const adcontrols::Chromatogram >
DataReader::TIC( int fcn ) const
{
    if ( tics_.empty() )
        const_cast< DataReader * >(this)->loadTICs();

    if ( tics_.size() > fcn )
        return tics_[ fcn ];

    return nullptr;
}

double
DataReader::time_since_inject( int64_t elapsed_time ) const
{
    if ( !indecies_.empty() )
        return ( elapsed_time - indecies_.front().elapsed_time ) * 1.0e-9;
    return -1.0;
}

void
DataReader::loadTICs()
{
    auto nfcn = fcnCount();

    std::map< int, std::pair< std::shared_ptr< adcontrols::Chromatogram >, uint64_t > > tics;
    
    if ( auto interpreter = interpreter_->_narrow< acqrsinterpreter::DataInterpreter >() ) {

        if ( auto db = db_.lock() ) {
            
            indecies_.clear();

            adfs::stmt sql( *db );
            
            sql.prepare( "SELECT npos,fcn,elapsed_time,data,meta FROM AcquiredData WHERE objuuid = ? ORDER BY npos" );
            sql.bind( 1 ) = objid_;
            
            while ( sql.step() == adfs::sqlite_row ) {
                
                auto pos = sql.get_column_value< int64_t >( 0 );
                auto fcn = int( sql.get_column_value< int64_t >( 1 ) );
                auto elapsed_time = sql.get_column_value< int64_t >( 2 ); // ns
                adfs::blob xdata = sql.get_column_value< adfs::blob >( 3 );
                adfs::blob xmeta = sql.get_column_value< adfs::blob >( 4 );

                indecies_.push_back( index( pos, elapsed_time, fcn ) );

                if ( tics.find( fcn ) == tics.end() )
                    tics[ fcn ] = std::make_pair( std::make_shared< adcontrols::Chromatogram >(), elapsed_time );

                auto pair = tics[ fcn ];

                waveform_types waveform;
                
                if ( interpreter->translate( waveform, xdata.data(), xdata.size(), xmeta.data(), xmeta.size() ) == adcontrols::translate_complete ) {

                    if ( pair.first->size() == 0 )
                        pair.first->addDescription( adcontrols::description( L"title", boost::apply_visitor( make_title(), waveform ).c_str() ) );

                    double d = boost::apply_visitor( total_ion_count(), waveform );
                    ( *pair.first ) << std::make_pair( double( elapsed_time - pair.second ) * 1.0e-9, d );

                }
            }

            for ( auto tic : tics )
                tics_.push_back( tic.second.first );
        }
    }
}

