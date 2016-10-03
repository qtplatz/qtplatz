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
#include "datainterpreter.hpp"
#include "datainterpreter_histogram.hpp"
#include "datainterpreter_timecount.hpp"
#include "datainterpreter_softavgr.hpp"
#include "datainterpreter_waveform.hpp"
#include <acqrscontrols/u5303a/threshold_result.hpp>
#include <acqrscontrols/ap240/threshold_result.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adcontrols/waveform.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
#include <adportable/utf.hpp>
#include <adutils/acquiredconf_v3.hpp>
#include <boost/format.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
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
        static const std::string display_name;
        typedef Interpreter type;
    };

    // "{ebec355c-3277-5b15-9430-b83031e7555c}" histogram.1.malpix.ms-cheminfo.com

    // u5303a
    template<> const std::string TID< waveform::DataInterpreter< acqrscontrols::u5303a::waveform> >::value = "1.u5303a.ms-cheminfo.com";
    template<> const std::string TID< waveform::DataInterpreter< acqrscontrols::u5303a::waveform> >::display_name = "1.u5303a";

    template<> const std::string TID< timecount::DataInterpreter< acqrscontrols::u5303a::threshold_result> >::value = "timecount.1.u5303a.ms-cheminfo.com";
    template<> const std::string TID< timecount::DataInterpreter<acqrscontrols::u5303a::threshold_result> >::display_name = "timecount[u5303a]";

    template<> const std::string TID< histogram::DataInterpreter >::value = "histogram.timecount.1.u5303a.ms-cheminfo.com";
    template<> const std::string TID< histogram::DataInterpreter >::display_name = "histogram";
    
    template<> const std::string TID< softavgr::DataInterpreter>::value = "tdcdoc.waveform.1.u5303a.ms-cheminfo.com";
    template<> const std::string TID< softavgr::DataInterpreter>::display_name = "waveform";

    // ap240
    // "{4f431f91-b08c-54ba-94f0-e1d13eba29d7}"
    template<> const std::string TID<timecount::DataInterpreter<acqrscontrols::ap240::threshold_result> >::value = "timecount.1.ap240.ms-cheminfo.com";
    template<> const std::string TID<timecount::DataInterpreter<acqrscontrols::ap240::threshold_result> >::display_name = "timecount[ap240]";

    // "{76d1f823-2680-5da7-89f2-4d2d956149bd}"
    template<> const std::string TID<waveform::DataInterpreter<acqrscontrols::ap240::waveform> >::value = "1.ap240.ms-cheminfo.com";
    template<> const std::string TID<waveform::DataInterpreter<acqrscontrols::ap240::waveform> >::display_name = "waveform[ap240]";

    // "{04c23c3c-7fd6-11e6-aa18-b7efcbc41dcd}", "1.dc122.ms-cheminfo.com"

    typedef boost::mpl::vector<
        TID< waveform::DataInterpreter<acqrscontrols::u5303a::waveform> >
        , TID< timecount::DataInterpreter<acqrscontrols::u5303a::threshold_result > >
        , TID< timecount::DataInterpreter<acqrscontrols::ap240::threshold_result > >
        , TID< waveform::DataInterpreter<acqrscontrols::ap240::waveform > >
        , TID< histogram::DataInterpreter >
        , TID< softavgr::DataInterpreter >
        > interpreter_types;

    //-------------------

    template< typename T > struct wrap {};

    struct make_trace_id_list {
        std::vector< std::string >& list;
        make_trace_id_list( std::vector< std::string >& _list ) : list(_list ){}
        template < typename T > void operator () ( wrap<T> ) const {
            list.push_back( T::value );
        }
    };
        
    struct lookup_and_create {
        const char * id;
        std::unique_ptr< adcontrols::DataInterpreter >& interpreter;
        std::string& display_name;
        lookup_and_create( const char * _id, std::unique_ptr< adcontrols::DataInterpreter >& t, std::string& name )
            : id( _id ), interpreter( t ), display_name( name )
            {}
        template < typename T > void operator () ( wrap<T> ) const {
            if ( id == T::value ) {
                interpreter = std::make_unique< typename T::type >();
                display_name = T::display_name;
            }
        }
    };

    struct total_ion_count : public boost::static_visitor< double > {
        double tof, width;
        total_ion_count( double _tof = 0.0, double _width = 0.0 ) : tof( _tof ), width( _width ) {}
        
        template< typename T > double operator()( T& ptr ) const { return ptr->accumulate( tof, width ); }
    };

    template<> double total_ion_count::operator()( std::shared_ptr< acqrscontrols::u5303a::threshold_result >& ptr ) const
    {
        return ptr->indecies2().size();
    }

    template<> double total_ion_count::operator()( std::shared_ptr< acqrscontrols::ap240::threshold_result >& ptr ) const
    {
        return ptr->indecies().size();
    }


    //------------------ coadd_spectrum visitor ----------------
    struct coadd_spectrum : public boost::static_visitor< void > {

        waveform_types& waveform;
        coadd_spectrum( waveform_types& _1 ) : waveform( _1 ) {}
        
        template< typename T > void operator()( T const& rhs ) const {
            auto ptr = boost::get< decltype( rhs ) >( waveform );
            *ptr += *rhs;
        }
    };

    template<> void coadd_spectrum::operator()( std::shared_ptr< acqrscontrols::u5303a::threshold_result > const& rhs ) const
    {
        // TBA
    }

    template<> void coadd_spectrum::operator()( std::shared_ptr< acqrscontrols::ap240::threshold_result > const& rhs ) const
    {
        if ( auto hgrm = boost::get< std::shared_ptr< adcontrols::TimeDigitalHistogram > >( waveform ) )
            *rhs >> *hgrm;
    }
    //------------------ coadd_spectrum visitor ----------------
    
    //------------------ coadd_initialize visitor ----------------
    struct coadd_initialize : public boost::static_visitor< void > {
        waveform_types& waveform;

        coadd_initialize( waveform_types& _1 ) : waveform( _1 ) {}

        template< typename T > void operator()( const T& rhs ) const {
            waveform = rhs;
        }
    };

    template<> void coadd_initialize::operator()( std::shared_ptr< acqrscontrols::ap240::threshold_result > const& rhs ) const
    {
        waveform = std::make_shared< adcontrols::TimeDigitalHistogram >();
        coadd_spectrum visit( waveform );
        visit( rhs );
    }
    
    //------------------ make_massspactrum visitor ----------------
    struct make_massspectrum : public boost::static_visitor< void > {
        adcontrols::MassSpectrum& ms;

        make_massspectrum( adcontrols::MassSpectrum& t ) : ms( t ) {}

        template< typename T > void operator () ( T const& waveform ) const {
            waveform->translate( ms, *waveform );
        }
    };

    template<> void make_massspectrum::operator()( std::shared_ptr< acqrscontrols::u5303a::threshold_result > const& rhs ) const
    {
        // TBA
    }

    template<> void make_massspectrum::operator()( std::shared_ptr< acqrscontrols::ap240::threshold_result > const& rhs ) const
    {
        // TBA
    }
    //------------------ make_massspactrum visitor ----------------

    //------------------ make_title visitor ----------------
    struct make_title : public boost::static_visitor < std::wstring > {
        std::wstring operator()( std::shared_ptr< acqrscontrols::u5303a::threshold_result> & ) const {
            return ( boost::wformat( L"U5303A-T" ) ).str();
        }
        std::wstring operator()( std::shared_ptr< adcontrols::TimeDigitalHistogram> & ) const {
            return ( boost::wformat( L"Histogram" ) ).str();
        }
        std::wstring operator()( std::shared_ptr< acqrscontrols::u5303a::waveform >& ) const {
            return ( boost::wformat( L"U5303A-A" ) ).str();
        }
        std::wstring operator()( std::shared_ptr< acqrscontrols::ap240::waveform >& ) const {
            return ( boost::wformat( L"AP240-A" ) ).str();
        }
        std::wstring operator()( std::shared_ptr< acqrscontrols::ap240::threshold_result> & ) const {
            return ( boost::wformat( L"AP240-T" ) ).str();
        }        
    };
    //------------------ make_title visitor ----------------

}

using namespace acqrsinterpreter;

DataReader::~DataReader()
{
    indecies_.clear();
    tics_.clear();
    interpreter_.reset();
}

DataReader::DataReader( const char * traceid ) : adcontrols::DataReader( traceid )
                                               , objid_( {{ 0 }} )
                                               , objrowid_( -1 )
                                               , fcnCount_( 0 )
{
    // traceid determines type of trace, a.k.a. type of mass-spectormeter, multi-dimentional chromatogram etc.
    // Though traceid does not indiecate trace object (in case two UV-ditectors on the system, traceid does not tell which one)

    if ( std::string( traceid ) == "1.dc122.ms-cheminfo.com" ) {
        interpreter_ = std::make_unique< waveform::DataInterpreter< acqrscontrols::ap240::waveform > >();
        display_name_ = "waveform[dc122]";
    } else {
        boost::mpl::for_each< interpreter_types
                              , wrap< boost::mpl::placeholders::_1> >( lookup_and_create( traceid, interpreter_, display_name_ ) );
    }
}

// static
std::vector< std::string >
DataReader::traceid_list()
{
    std::vector< std::string > list;
    boost::mpl::for_each< interpreter_types, wrap< boost::mpl::placeholders::_1> >( make_trace_id_list( list ) );
    list.emplace_back( "1.dc122.ms-cheminfo.com" );
    return list;
}

bool
DataReader::initialize( adfs::filesystem& dbf, const boost::uuids::uuid& objid, const std::string& objtext )
{
    if ( interpreter_ ) {
        
        objid_ = objid; // objid tells channel/module id
        objtext_ = objtext; // for debugging convension
        db_ = dbf._ptr();

        if ( auto db = db_.lock() ) {
            {
                adfs::stmt sql( *db );
                sql.prepare( "SELECT rowid FROM AcquiredConf WHERE objuuid = ?" );
                sql.bind( 1 ) = objid_;
                if ( sql.step() == adfs::sqlite_row )
                    objrowid_ = sql.get_column_value< int64_t >( 0 );
            }
            // fcnCount
            {
                adfs::stmt sql( *db );
                sql.prepare( "SELECT COUNT( DISTINCT fcn ) FROM AcquiredData WHERE objuuid = ?" );
                sql.bind( 1 ) = objid_;
                if ( sql.step() == adfs::sqlite_row )
                    fcnCount_ = sql.get_column_value< int64_t >( 0 );
            }
            
            // find ScanLaw
            double acclVoltage( 0 ), tDelay( 0 ), fLength;
            boost::uuids::uuid clsid { 0 };
            if ( adutils::v3::AcquiredConf::findScanLaw( *db, objid_, clsid, acclVoltage, tDelay, fLength ) ) {
                // src/adplugins/adspectrometer/massspectrometer.hpp; "adspectrometer"
                // clsid = boost::uuids::string_generator()( "{E45D27E0-8478-414C-B33D-246F76CF62AD}" );
                // acclVoltage = 5000.0;
                if ( ( spectrometer_ = adcontrols::MassSpectrometerBroker::make_massspectrometer( clsid ) ) )
                    spectrometer_->setScanLaw( acclVoltage, tDelay, fLength );
            }

            // workaround for Sep. to Dec., 2015 data file
            // find if protocol override exist
            {
                bool exists( false );
                {
                    adfs::stmt sql( *db );
                    sql.prepare( "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='PROTOCOL_OVERRIDE'" );
                    if ( sql.step() == adfs::sqlite_row )
                        exists = sql.get_column_value<int64_t>( 0 );
                };
                
                if ( exists ) {
                    std::vector< std::pair< int, double > > protocols;
                    adfs::stmt sql( *db );
                    sql.prepare( "SELECT  mode, ext_adc_delay from PROTOCOL_OVERRIDE ORDER BY id" );
                    while ( sql.step() == adfs::sqlite_row ) {
                        int mode = int( sql.get_column_value< uint64_t >( 0 ) );
                        double delay = sql.get_column_value< double >( 1 );
                        protocols.emplace_back( mode, delay );
                    }
                    if ( ! protocols.empty() ) {
                        if ( auto ip = dynamic_cast< acqrsinterpreter::DataInterpreter * >( interpreter_.get() ) )
                            ip->setWorkaroundProtocols( protocols );
                    }
                }
            }
            // end workaround
        }
        return true;
    }
    ADDEBUG() << "initialize failed for: " << objtext;
    return false;
}

void
DataReader::finalize()
{
}


const boost::uuids::uuid&
DataReader::objuuid() const
{
    return objid_;
}

const std::string&
DataReader::objtext() const
{
    return objtext_;
}

int64_t
DataReader::objrowid() const
{
    return objrowid_;
}

const std::string&
DataReader::display_name() const
{
    return display_name_;
}

size_t
DataReader::fcnCount() const
{
    // skip timecount data -- too large to handle in the dataproc
    if ( auto i = interpreter_->_narrow< timecount::DataInterpreter<acqrscontrols::u5303a::threshold_result> >() ) {
         (void)i;
         return 0;
    }
    return fcnCount_;
}

adcontrols::DataReader::const_iterator
DataReader::begin( int fcn ) const
{
    if ( indecies_.empty() ) {
        
        if ( auto db = db_.lock() ) {
            adfs::stmt sql( *db );
            if ( fcn >= 0 ) {
                sql.prepare( "SELECT rowid FROM AcquiredData WHERE objuuid=? AND fcn=? ORDER BY npos LIMIT 1" );
                sql.bind( 1 ) = objid_;
            } else {
                sql.prepare( "SELECT rowid FROM AcquiredData WHERE objuuid=? ORDER BY npos LIMIT 1" );
                sql.bind( 1 ) = objid_;
            }
            if ( sql.step() == adfs::sqlite_row ) {
                auto rowid = sql.get_column_value< int64_t >( 0 );
                return adcontrols::DataReader_iterator( this, rowid, fcn );
            }
        }
    }
    return adcontrols::DataReader_iterator( this, next( 0, fcn ), fcn );
}

adcontrols::DataReader::const_iterator
DataReader::end() const
{
    return adcontrols::DataReader_iterator( this, (-1) );
}

adcontrols::DataReader::const_iterator
DataReader::findPos( double seconds, int fcn, bool closest, TimeSpec tspec ) const
{
    ADDEBUG() << "findPos( " << int64_t( seconds * std::nano::den ) << ")";
    
    if ( tspec == ElapsedTime ) {    
        if ( auto db = db_.lock() ) {
            adfs::stmt sql( *db );
            sql.prepare(
                "SELECT rowid FROM AcquiredData WHERE "
                "objuuid=:objid "
                "AND fcn=? "
                "AND elapsed_time >= (SELECT MIN(elapsed_time) FROM AcquiredData WHERE objuuid=:objid) + ? LIMIT 1");
            sql.bind( 1 ) = objid_;
            sql.bind( 2 ) = ( fcn < 0 ? 0 : fcn );
            sql.bind( 3 ) = uint64_t( seconds * std::nano::den );

            if ( sql.step() == adfs::sqlite_row ) {
                auto rowid = sql.get_column_value< int64_t >( 0 );
                return adcontrols::DataReader_iterator( this, rowid, fcn );
            }
        }
#if 0
        // compute from indecies_
        if ( tspec == ElapsedTime ) {

        int64_t elapsed_time = int64_t( seconds * 1e9 + 0.5 ) + indecies_.front().elapsed_time;

        if ( indecies_.front().elapsed_time > elapsed_time )
            return begin( (-1) );

        if ( indecies_.back().elapsed_time < elapsed_time )
            return adcontrols::DataReader_iterator( this, indecies_.back().rowid );

        auto it = std::lower_bound( indecies_.begin(), indecies_.end(), elapsed_time, [] ( const index& a, int64_t b ) { return a.elapsed_time < b; } );

        if ( closest && ( it != indecies_.end() ) ) {
            if ( std::abs( elapsed_time - it->elapsed_time ) > std::abs( elapsed_time - ( it + 1 )->elapsed_time ) ) 
                ++it;
        }

        return adcontrols::DataReader_iterator( this, it->rowid );
#endif
    }

    return end();
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

void
DataReader::loadTICs()
{
    auto nfcn = fcnCount();

    std::map< int, std::pair< std::shared_ptr< adcontrols::Chromatogram >, uint64_t > > tics;
    
    if ( auto interpreter = interpreter_->_narrow< acqrsinterpreter::DataInterpreter >() ) {

        if ( auto db = db_.lock() ) {
            
            indecies_.clear();

            adfs::stmt sql( *db );
            
            sql.prepare( "SELECT rowid,npos,fcn,elapsed_time,data,meta FROM AcquiredData WHERE objuuid = ? ORDER BY npos,fcn" );
            sql.bind( 1 ) = objid_;
            
            while ( sql.step() == adfs::sqlite_row ) {
                
                int col = 0;
                auto row = sql.get_column_value< int64_t >( col++ );
                auto pos = sql.get_column_value< int64_t >( col++ );
                auto fcn = int( sql.get_column_value< int64_t >( col++ ) );
                auto elapsed_time = sql.get_column_value< int64_t >( col++ ); // ns
                adfs::blob xdata = sql.get_column_value< adfs::blob >( col++ );
                adfs::blob xmeta = sql.get_column_value< adfs::blob >( col++ );

                indecies_.emplace_back( row, pos, elapsed_time, fcn ); // <-- struct index

                if ( tics.find( fcn ) == tics.end() ) {
                    tics [ fcn ] = std::make_pair( std::make_shared< adcontrols::Chromatogram >(), elapsed_time );
                    tics [ fcn ].first->setDataReaderUuid( objid_ );
                    tics [ fcn ].first->setFcn( fcn );
                }

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

int64_t
DataReader::next( int64_t rowid ) const
{
    auto it = std::lower_bound( indecies_.begin(), indecies_.end(), rowid, [] ( const index& a, int64_t rowid ) { return a.rowid < rowid; } );
    if ( it != indecies_.end() && ++it != indecies_.end() )
        return it->rowid;
    return -1;
}

int64_t
DataReader::next( int64_t rowid, int fcn ) const
{
    if ( fcn == ( -1 ) )
        return next( rowid );

    auto it = std::lower_bound( indecies_.begin(), indecies_.end(), rowid, [] ( const index& a, int64_t rowid ) { return a.rowid < rowid; } );
    if ( it != indecies_.end() ) {
        while ( ++it != indecies_.end() )
            if ( it->fcn == fcn )
                return it->rowid;
    }
    return (-1);
}

int64_t
DataReader::pos( int64_t rowid ) const
{
    auto it = std::lower_bound( indecies_.begin(), indecies_.end(), rowid, [] ( const index& a, int64_t rowid ) { return a.rowid < rowid; } );
    if ( it != indecies_.end() )
        return it->pos;
    return -1;
}

int64_t
DataReader::elapsed_time( int64_t rowid ) const
{
    auto it = std::lower_bound( indecies_.begin(), indecies_.end(), rowid, [] ( const index& a, int64_t rowid ) { return a.rowid < rowid; } );
    if ( it != indecies_.end() )
        return it->elapsed_time;
    return -1;    
}

double
DataReader::time_since_inject( int64_t rowid ) const
{
    auto it = std::lower_bound( indecies_.begin(), indecies_.end(), rowid, [] ( const index& a, int64_t rowid ) { return a.rowid < rowid; } );
    if ( it != indecies_.end() )
        return double( it->elapsed_time - indecies_.front().elapsed_time ) * 1.0e-9;
    return -1;        
}

int
DataReader::fcn( int64_t rowid ) const
{
    auto it = std::lower_bound( indecies_.begin(), indecies_.end(), rowid, [] ( const index& a, int64_t rowid ) { return a.rowid < rowid; } );
    if ( it != indecies_.end() )
        return it->fcn;
    return -1;    
}

std::shared_ptr< adcontrols::MassSpectrum >
DataReader::getSpectrum( int64_t rowid ) const
{
    if ( auto interpreter = interpreter_->_narrow< acqrsinterpreter::DataInterpreter >() ) {
    
        if ( auto db = db_.lock() ) {
     
            adfs::stmt sql( *db );
            
            if ( sql.prepare( "SELECT data, meta FROM AcquiredData WHERE rowid = ?" ) ) {
                sql.bind( 1 ) = rowid;
         
                if ( sql.step() == adfs::sqlite_row ) {
             
                    adfs::blob xdata = sql.get_column_value< adfs::blob >( 0 );
                    adfs::blob xmeta = sql.get_column_value< adfs::blob >( 1 );

                    waveform_types waveform;
                    if ( interpreter->translate( waveform, xdata.data(), xdata.size()
                                                 , xmeta.data(), xmeta.size() ) == adcontrols::translate_complete ) {

                        auto ptr = std::make_shared< adcontrols::MassSpectrum >();
                        boost::apply_visitor( make_massspectrum( *ptr ), waveform );
                        if ( spectrometer_ ) {
                            spectrometer_->assignMasses( *ptr );
                            const auto& info = ptr->getMSProperty().samplingInfo();
                            double lMass = spectrometer_->scanLaw()->getMass( info.fSampDelay(), int( info.mode() ) );
                            double uMass = spectrometer_->scanLaw()->getMass( info.fSampDelay() + info.nSamples() * info.fSampInterval(), int( info.mode() ) );
                            ptr->setAcquisitionMassRange( lMass, uMass );
                        }
                        ptr->addDescription( adcontrols::description( L"title", boost::apply_visitor( make_title(), waveform ).c_str() ) );
                        ptr->setDataReaderUuid( objid_ );
                        ptr->setRowid( rowid );

                        return ptr;
                    }
                }
            }
        }
    }
    return nullptr;
}

std::shared_ptr< adcontrols::MassSpectrum >
DataReader::readSpectrum( const_iterator& it ) const
{
    if ( it._fcn() >= 0 )
        return getSpectrum( it->rowid() );

    if ( auto interpreter = interpreter_->_narrow< acqrsinterpreter::DataInterpreter >() ) {    
        if ( auto db = db_.lock() ) {
            
            adfs::stmt sql( *db );
            sql.prepare( "SELECT rowid,fcn,data,meta FROM AcquiredData WHERE objuuid = ? AND npos >= ? ORDER BY npos LIMIT ?" );
            sql.bind( 1 ) = objid_;
            sql.bind( 2 ) = it->pos();
            sql.bind( 3 ) = fcnCount();
            
            std::shared_ptr< adcontrols::MassSpectrum > prime;
            
            while ( sql.step() == adfs::sqlite_row ) {
                auto ptr = std::make_shared< adcontrols::MassSpectrum >();
                int col = 0;
                auto rowid = sql.get_column_value< int64_t >( col++ );
                auto _fno  = sql.get_column_value< int64_t >( col++ );
                adfs::blob xdata = sql.get_column_value< adfs::blob >( col++ );
                adfs::blob xmeta = sql.get_column_value< adfs::blob >( col++ );
                
                waveform_types waveform;
                if ( interpreter->translate( waveform, xdata.data(), xdata.size()
                                             , xmeta.data(), xmeta.size() ) == adcontrols::translate_complete ) {
                    
                    boost::apply_visitor( make_massspectrum( *ptr ), waveform );
                    if ( spectrometer_ ) {
                        spectrometer_->assignMasses( *ptr );
                        const auto& info = ptr->getMSProperty().samplingInfo();
                        double lMass = spectrometer_->scanLaw()->getMass( info.fSampDelay(), int( info.mode() ) );
                        double uMass = spectrometer_->scanLaw()->getMass( info.fSampDelay() + info.nSamples() * info.fSampInterval(), int( info.mode() ) );
                        ptr->setAcquisitionMassRange( lMass, uMass );
                    }
                    ptr->addDescription( adcontrols::description( L"title", boost::apply_visitor( make_title(), waveform ).c_str() ) );
                    ptr->setDataReaderUuid( objid_ );
                    ptr->setRowid( rowid );
                }
                if ( !prime )
                    prime = ptr;
                else
                    (*prime) << std::move( ptr );
            }

            return prime;
        }
    }
    return nullptr;
}

std::shared_ptr< adcontrols::Chromatogram >
DataReader::getChromatogram( int fcn, double time, double width ) const
{
    auto nfcn = fcnCount();
    if ( fcn >= nfcn )
        return nullptr;

    auto ptr = std::make_shared< adcontrols::Chromatogram >();
    ptr->setDataReaderUuid( objid_ );

    if ( auto interpreter = interpreter_->_narrow< acqrsinterpreter::DataInterpreter >() ) {

        if ( auto db = db_.lock() ) {
            
            adfs::stmt sql( *db );
            
            sql.prepare( "SELECT elapsed_time,data,meta FROM AcquiredData WHERE objuuid = ? AND fcn = ? ORDER BY npos" );
            sql.bind( 1 ) = objid_;
            sql.bind( 2 ) = fcn;
            double t0(0);
            
            while ( sql.step() == adfs::sqlite_row ) {
                
                int col = 0;
                
                auto elapsed_time = sql.get_column_value< int64_t >( col++ ); // ns
                adfs::blob xdata = sql.get_column_value< adfs::blob >( col++ );
                adfs::blob xmeta = sql.get_column_value< adfs::blob >( col++ );

                waveform_types waveform;
                
                if ( interpreter->translate( waveform, xdata.data(), xdata.size(), xmeta.data(), xmeta.size() ) == adcontrols::translate_complete ) {

                    if ( ptr->size() == 0 ) {
                        t0 = elapsed_time;
                        ptr->addDescription( adcontrols::description( L"title", boost::apply_visitor( make_title(), waveform ).c_str() ) );
                    }
                    
                    double d = boost::apply_visitor( total_ion_count( time, width ), waveform );
                    *ptr << std::make_pair( double( elapsed_time - t0 ) * 1.0e-9, d );
                    
                }
            }

            return ptr;
        }
    }
    return nullptr;    
}

std::shared_ptr< adcontrols::MassSpectrum >
DataReader::coaddSpectrum( const_iterator& begin, const_iterator& end ) const
{
    if ( auto interpreter = interpreter_->_narrow< acqrsinterpreter::DataInterpreter >() ) {

        if ( auto db = db_.lock() ) {
            
            adfs::stmt sql( *db );
            
            int fcn = begin._fcn(); // if this is -1, query all protocols

            if ( fcn < 0 ) {
                sql.prepare( "SELECT elapsed_time,fcn,data,meta FROM AcquiredData WHERE objuuid = ? AND npos >= ? AND npos <= ? ORDER BY npos" );
                sql.bind( 1 ) = objid_;
                sql.bind( 2 ) = begin->pos();
                sql.bind( 3 ) = end->pos();
            } else {
                sql.prepare( "SELECT elapsed_time,fcn,data,meta FROM AcquiredData WHERE objuuid = ? AND fcn = ? AND npos >= ? AND npos <= ? ORDER BY npos" );
                sql.bind( 1 ) = objid_;
                sql.bind( 2 ) = fcn;
                sql.bind( 3 ) = begin->pos();
                sql.bind( 4 ) = end->pos();
            }

            std::map< int, std::pair< size_t, waveform_types > > coadded;
            auto ptr = std::make_shared< adcontrols::MassSpectrum >();
            
            // waveform_types coadded;
            
            size_t n(0);
            while ( sql.step() == adfs::sqlite_row ) {
                
                int col = 0;
                auto elapsed_time = sql.get_column_value< int64_t >( col++ ); // ns
                auto _fno = sql.get_column_value< int64_t >( col++ );
                adfs::blob xdata = sql.get_column_value< adfs::blob >( col++ );
                adfs::blob xmeta = sql.get_column_value< adfs::blob >( col++ );
                
                waveform_types waveform;
                if ( interpreter->translate( waveform, xdata.data(), xdata.size()
                                             , xmeta.data(), xmeta.size() ) == adcontrols::translate_complete ) {

                    if ( coadded[ _fno ].first++ == 0 ) {
                        ptr->addDescription( adcontrols::description( L"title", boost::apply_visitor( make_title(), waveform ).c_str() ) );
                        boost::apply_visitor( coadd_initialize( coadded[ _fno ].second ), waveform );
                        // coadded = waveform;
                    } else {
                        boost::apply_visitor( coadd_spectrum( coadded[ _fno ].second ), waveform );
                    }
                }
            }
            
            int count(0);
            for ( const auto& wform: coadded ) {
                
                auto ms = ( count == 0 ) ? ptr : std::make_shared< adcontrols::MassSpectrum >();
                ms->setDataReaderUuid( objid_ );

                boost::apply_visitor( make_massspectrum( *ms ), wform.second.second );
                ADDEBUG() << "protocolId = " << ms->protocolId() << "== fcn=" << wform.first;

                if ( spectrometer_ ) {
                    spectrometer_->assignMasses( *ms );
                    const auto& info = ms->getMSProperty().samplingInfo();
                    double lMass = spectrometer_->scanLaw()->getMass( info.fSampDelay(), int( info.mode() ) );
                    double uMass = spectrometer_->scanLaw()->getMass( info.fSampDelay() + info.nSamples() * info.fSampInterval(), int( info.mode() ) );
                    ms->setAcquisitionMassRange( lMass, uMass );
                }
                if ( count > 0 )
                    (*ptr) << std::move( ms );
                ++count;
            }
            
            return ptr;
        }
    }
    return nullptr;    
}

std::shared_ptr< adcontrols::MassSpectrometer >
DataReader::massSpectrometer() const
{
    return spectrometer_;
}
