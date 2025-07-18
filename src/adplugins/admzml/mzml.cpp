// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC
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

#include "mzml.hpp"
#include "accession.hpp"
#include "mzmlwalker.hpp"
#include "mzmlchromatogram.hpp"
#include "mzmlspectrum.hpp"
#include "scan_protocol.hpp"
#include "xmltojson.hpp"
#include "datareader.hpp"
#include <pugixml.hpp>
#include <adacquire/signalobserver.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/lockmass.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/msfractuation.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/traceaccessor.hpp>
#include <adfs/adfs.hpp>
#include <adfs/sqlite.hpp>
#include <adlog/logger.hpp>
#include <adplugin_manager/manager.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/debug.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/utf.hpp>
#include <adutils/mscalibio.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <numeric>
#include <sstream>
#include <unordered_map>
#include <variant>

namespace {
    // helper for visitor
    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;
    // end helper for visitor
}

namespace mzml {

    class mzML::impl {
        std::mutex mutex_;
    public:
        pugi::xml_document doc_;
        std::optional< fileDescription > fileDescription_;
        std::optional< softwareList > softwareList_;
        std::optional< instrumentConfigurationList > instrumentConfigurationList_;
        std::optional< dataProcessingList > dataProcessingList_;
        spectra_t spectra_; // all spectra stored in the file including no length data
        chromatograms_t chromatograms_;
        std::map< int, std::shared_ptr< adcontrols::Chromatogram > > TICs_;
        std::map< int, std::shared_ptr< adcontrols::Chromatogram > > SRMs_; // both SRM and SIM
        std::map< int, pugi::xml_node > srm_node_; // both SRM and SIM
        std::map< int, std::shared_ptr< local::data_reader > > dataReaders_;

        std::vector< std::pair< mzml::scan_id, std::shared_ptr<mzml::mzMLSpectrum> > > scan_indices_;
        std::unordered_map< mzml::scan_protocol_key_t, int, mzml::protocol_key_hash, mzml::protocol_key_equal > protocol_map_;
        std::unordered_map< mzml::scan_protocol_key_t, int, mzml::protocol_key_hash, mzml::protocol_key_equal > srm_map_;
        size_t number_of_protocols_;

        impl() : number_of_protocols_( 0 ) {}

        //-------------------------
        // std::shared_ptr< adcontrols::DataReader > dataReader( const mzML * p ) {
        //     if ( not dataReader_ ) {
        //         // std::unique_lock lock( mutex_ );
        //         if ( not dataReader_ )
        //             dataReader_ = std::make_shared< DataReader >( "mzML", p->shared_from_this() );
        //     }
        //     return dataReader_;
        // }

        //-------------------------
        void print() {
            if ( auto p = fileDescription_ ) {
                ADDEBUG() << boost::json::value_from( *p );
            }
            if ( auto p = softwareList_ ) {
                ADDEBUG() << boost::json::value_from( *p );
            }
            if ( auto p = instrumentConfigurationList_ ) {
                ADDEBUG() << boost::json::value_from( *p );
            }
            if ( auto p = dataProcessingList_ ) {
                // p->node().print( std::cout );
            }
        }

        void handle_SRM( std::shared_ptr< const mzml::mzMLSpectrum > sp, int& next_id ) {
            auto scan_protocol = std::get< enum_scan_protocol >( mzml::scan_identifier()( sp->node() ) );
            auto key = std::get< enum_scan_protocol >( mzml::scan_identifier()( sp->node() ) ).protocol_key();
            auto [ it, inserted ] = srm_map_.emplace( key, next_id );
            if ( inserted ) {
                std::ostringstream xml;
                sp->node().print( xml );
                SRMs_[ it->second ] = std::make_shared< adcontrols::Chromatogram >();
                srm_node_[ it->second ] = sp->node();

                if ( scan_protocol.ms_level() == 2 ) {
                    SRMs_[ it->second ]->addDescription( { "id"
                            , (boost::format("SRM %.1f->%.1f %s %.1fV")
                               % scan_protocol.precursor_mz()
                               % sp->base_peak().first
                               % (scan_protocol.polarity() == polarity_negative ? "(neg)" : "(pos)")
                               % scan_protocol.collision_energy()).str() } );
                } else {
                    SRMs_[ it->second ]->addDescription( { "id"
                            , (boost::format("SIM %.1f %s")
                               % sp->base_peak().first
                               % (scan_protocol.polarity() == polarity_negative ? "(neg)" : "(pos)") ).str() });
                }
                next_id++;
            }
            (*SRMs_[ it->second ]) << std::make_pair( sp->scan_start_time(), sp->base_peak().second );
        }

        //------------------------
        void make_scan_indices( const mzML * mzml ) {
            int next_id{0}, next_srm_id(0);
            for ( const auto& sp: spectra_ ) {
                if ( sp->length() > 0 ) {
                    auto id = mzml::scan_identifier()( sp->node() );
                    auto key  = std::get< enum_scan_protocol >( id ).protocol_key();
                    auto [it, inserted] = protocol_map_.emplace( key, next_id );
                    if ( inserted ) {
                        TICs_[ it->second ] = std::make_shared< adcontrols::Chromatogram >();
                        TICs_[ it->second ]->addDescription( { "id", (boost::format("TIC/TIC.%d") % (it->second + 1)).str() } );
                        auto traceid = boost::json::value_from( std::get< enum_scan_protocol >( id ) );
                        dataReaders_[ it->second ] = std::make_shared< local::data_reader >( boost::json::serialize( traceid ).c_str()
                                                                                             , it->second // fcn
                                                                                             , mzml->shared_from_this() );
                        ++next_id;
                    }
                    sp->set_protocol_id( it->second );
                    (*TICs_[ it->second ]) << std::make_pair( sp->scan_start_time(), sp->total_ion_current() );
                    scan_indices_.emplace_back( id, sp );
                } else {
                    handle_SRM( sp, next_srm_id );
                }
            }
            for ( auto& cp: TICs_ ) {
                cp.second->setMinimumTime( cp.second->timeArray().front() );
                cp.second->setMaximumTime( cp.second->timeArray().back() );
            }
            for ( auto& cp: SRMs_ ) {
                cp.second->setMinimumTime( cp.second->timeArray().front() );
                cp.second->setMaximumTime( cp.second->timeArray().back() );
            }

            number_of_protocols_ = next_id;
        }
    };


}

using namespace mzml;

mzML::~mzML()
{
}

mzML::mzML() : impl_( std::make_unique< mzml::mzML::impl >() )
{
}

const std::map< int, std::shared_ptr< local::data_reader > >
mzML::dataReaderMap() const
{
    return impl_->dataReaders_;
}

bool
mzML::open( const std::filesystem::path& path )
{
    if ( auto result = impl_->doc_.load_file( path.c_str() ) ) {
        if ( auto node = impl_->doc_.select_node( "/indexedmzML" ) ) {
            mzml::mzMLWalker walker;

            auto var = walker( node.node() );
            for ( auto& data: var ) {
                std::visit( overloaded {
                        [&](fileDescription& arg) {
                            impl_->fileDescription_ = std::move( arg );
                        }
                            , [&](softwareList& arg) {
                                impl_->softwareList_ = std::move( arg );
                            }
                            , [&](instrumentConfigurationList& arg) {
                                impl_->instrumentConfigurationList_ = std::move( arg );
                            }
                            , [&](dataProcessingList& arg) {
                                impl_->dataProcessingList_ = std::move( arg );
                            }
                            , [&]( const spectra_t& arg ){
                                impl_->spectra_ = std::move( arg );
                            }
                            , [&](const chromatograms_t& arg ) {
                                impl_->chromatograms_ = std::move( arg );
                            }
                            }, data );
            }
            // impl_->print();
            impl_->make_scan_indices( this );
            return true;
        }
    }
    return false;
}

std::vector< std::shared_ptr< adcontrols::Chromatogram > >
mzML::import_chromatograms() const
{
    std::vector< std::shared_ptr< adcontrols::Chromatogram > > vec;
    for ( const auto& tic: impl_->TICs_ )
        vec.emplace_back( tic.second );
#if 0
    // Shimadzu LabSolutions saved TIC&BPC chromatograms, which contains SRM or SIM data so that no need to review
    for ( const auto& pc: impl_->chromatograms_ )
        vec.emplace_back( mzMLChromatogram::toChromatogram( *pc ) );
#endif
    for ( const auto& srm: impl_->SRMs_ )
        vec.emplace_back( srm.second );
    return vec;
}

std::vector< std::tuple< int, std::shared_ptr< const adcontrols::Chromatogram >, pugi::xml_node > >
mzML::SRMs() const
{
    std::vector< std::tuple< int, std::shared_ptr< const adcontrols::Chromatogram >, pugi::xml_node > > vec;
    for ( auto& srm: impl_->SRMs_ ) {
        auto it = impl_->srm_node_.find( srm.first );
        if ( it != impl_->srm_node_.end() ) {
            auto scan_protocol = std::get< enum_scan_protocol >(mzml::scan_identifier{}( it->second ));
            ADDEBUG() << "SRMs fcn: " << srm.first << ", " << scan_protocol;
            vec.emplace_back( srm.first, srm.second, it->second );
        }
    }
    return vec;
}

std::vector< std::tuple< int, std::shared_ptr< const adcontrols::Chromatogram > > >
mzML::TICs() const
{
    std::vector< std::tuple< int, std::shared_ptr< const adcontrols::Chromatogram > > > vec;
    for ( auto [fcn,tic]: impl_->TICs_ ) {
        vec.emplace_back( fcn, tic );
    }
    return vec;
}

std::vector< std::shared_ptr< mzml::mzMLChromatogram > >
mzML::mzMLChromatograms() const
{
    return impl_->chromatograms_;
}

const std::vector< std::pair< mzml::scan_id, std::shared_ptr< mzMLSpectrum > > > &
mzML::scan_indices() const
{
    return impl_->scan_indices_;
}

int
mzML::get_protocol_index( const mzml::scan_protocol_key_t& key ) const
{
    auto it = impl_->protocol_map_.find( key );
    if (it != impl_->protocol_map_.end() )
        return it->second;
    return -1;
}

std::optional<mzml::scan_protocol_key_t>
mzML::find_key_by_index(int index) const
{
    for (const auto& [key, idx] : impl_->protocol_map_) {
        if (idx == index)
            return key;
    }
    return {};
}


std::pair< mzml::scan_id, std::shared_ptr< const mzml::mzMLSpectrum > >
mzML::find_spectrum( int fcn, size_t pos, size_t rowid ) const
{
    ADDEBUG() << "find_spectrum: " << std::make_tuple( fcn, pos, rowid );
    // if ( fcn >= 0 ) {
        // if ( auto tkey = find_key_by_index( fcn ) ) {
        //     for ( const auto& idx: impl_->scan_indices_ ) {
        //         ;
        //     }
        // }
    if ( impl_->scan_indices_.size() > rowid ) {
        return impl_->scan_indices_.at( rowid );
    }
    return {};
}

std::optional< std::pair< mzml::scan_id, std::shared_ptr< const mzml::mzMLSpectrum > > >
mzML::find_first_spectrum( int fcn, double tR ) const
{
    if ( fcn >= 0 ) {
        if ( auto key = find_key_by_index( fcn ) ) {
            auto it = std::lower_bound( impl_->scan_indices_.begin()
                                        , impl_->scan_indices_.end()
                                        , tR
                                        , [&](const auto& a, const auto& b) {
                                            return std::get< enum_scan_start_time >(a.first) < b &&
                                                std::get< enum_scan_protocol >(a.first).protocol_key() == *key;
                                        });
            if ( it != impl_->scan_indices_.end() )
                return *it;
        }
    }
    return {};
}

const pugi::xml_document&
mzML::xml_document() const
{
    return impl_->doc_;
}

std::optional< fileDescription >
mzML::get_fileDescription() const
{
    return impl_->fileDescription_;
}

std::optional< softwareList >
mzML::get_softwareList() const
{
    return impl_->softwareList_;
}

std::optional< instrumentConfigurationList >
mzML::get_instrumentConfigurationList() const
{
    return impl_->instrumentConfigurationList_;
}

std::optional< dataProcessingList >
mzML::get_dataProcessingList() const
{
    return impl_->dataProcessingList_;
}

//////////
int
mzML::dataformat_version() const
{
    return 3;
}

// LCMSDataset();
size_t
mzML::getFunctionCount() const
{
    return impl_->number_of_protocols_;
}

size_t
mzML::getSpectrumCount( int fcn ) const
{
    return std::accumulate( impl_->scan_indices_.begin()
                     , impl_->scan_indices_.end()
                     , size_t{}
                     , [&](size_t a, const auto& b) {
                         return a + ( b.second->protocol_id() == fcn ? 1 : 0 );
                     });
}

size_t
mzML::getChromatogramCount() const
{
    return impl_->TICs_.size() + impl_->SRMs_.size() + impl_->chromatograms_.size();
}

bool
mzML::getTIC( int fcn, adcontrols::Chromatogram& ) const
{
    return {};
}

bool
mzML::getSpectrum( int fcn, size_t npos, adcontrols::MassSpectrum& ms, uint32_t objid ) const
{
    ADDEBUG() << "getSpectrum(" << std::make_tuple( fcn, npos, objid ) << ")";
    return true;
}

size_t
mzML::posFromTime( double x ) const
{
    return {};
}

double
mzML::timeFromPos( size_t ) const
{
    return {};
}

bool
mzML::index( size_t /*pos*/, int& /*idx*/, int& /*fcn*/, int& /*rep*/, double * t ) const
{
    return false; (void)t;
}

size_t
mzML::find_scan( int idx, int /* fcn */ ) const
{
    return idx;
}

int /* idx */
mzML::make_index( size_t pos, int& fcn ) const
{
    fcn = 0;
    return int(pos);
}

bool
mzML::getChromatograms( const std::vector< std::tuple<int, double, double> >&
                        , std::vector< adcontrols::Chromatogram >&
                        , std::function< bool (long curr, long total ) > progress
                        , int begPos
                        , int endPos ) const
{
    return false;
}


// v3 data support
size_t
mzML::dataReaderCount() const
{
    return 0;
}

const adcontrols::DataReader *
mzML::dataReader( size_t idx ) const
{
    ADDEBUG() << "##################### NOT IMPLEMENTED ###########################";
    // if ( idx == 0 )
    //     return impl_->dataReader( this ).get();
    return nullptr;

}

const adcontrols::DataReader *
mzML::dataReader( const boost::uuids::uuid& ) const
{
    ADDEBUG() << "##################### NOT IMPLEMENTED ###########################";
    return nullptr; // impl_->dataReader( this ).get();
}

std::vector < std::shared_ptr< adcontrols::DataReader > >
mzML::dataReaders( bool allPossible ) const
{
    std::vector < std::shared_ptr< adcontrols::DataReader > > vec;
    for ( auto a: impl_->dataReaders_ )
        vec.emplace_back( a.second );
    return vec;
    // try {
    //     return std::vector < std::shared_ptr< adcontrols::DataReader > >{ impl_->dataReader( this ) };
    // } catch ( std::exception& ex ) {
    //     ADDEBUG() << "## Exception: " << ex.what();
    // }
    // return {};
}

adcontrols::MSFractuation *
mzML::msFractuation() const
{
    return nullptr;
}


#if 0
mzML::mzML( adfs::filesystem& dbf
            , adcontrols::datafile& parent ) : dbf_( dbf )
                                             , fcnCount_( 0 )
                                             , npos0_( 0 )
                                             , configLoaded_( false )
{
}

adfs::sqlite *
mzML::db() const
{
    return &dbf_.db();
}

bool
mzML::loadAcquiredConf()
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
mzML::loadMSFractuation()
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
mzML::dataReaderCount() const
{
    return readers_.size();
}

const adcontrols::DataReader *
mzML::dataReader( size_t idx ) const
{
    if ( idx < readers_.size() )
        return readers_ [ idx ].first.get();
    return nullptr;
}

const adcontrols::DataReader *
mzML::dataReader( const boost::uuids::uuid& uuid ) const
{
    typedef std::pair < std::shared_ptr< adcontrols::DataReader >, int> value_type;

    auto it = std::find_if( readers_.begin(), readers_.end(), [&] ( const value_type& a ) { return a.first->objuuid() == uuid; } );
    if ( it != readers_.end() )
        return it->first.get();
    return nullptr;
}

std::vector < std::shared_ptr< adcontrols::DataReader > >
mzML::dataReaders( bool allPossible ) const
{
    std::vector < std::shared_ptr< adcontrols::DataReader > > v;
    v.reserve( readers_.size() );

    if ( allPossible ) {
        for ( auto& reader : readers_ )
            v.emplace_back( reader.first );
    } else {
        for ( auto& reader : readers_ )
            if ( reader.first->fcnCount() > 0 || reader.first->trace_method() == adacquire::SignalObserver::eTRACE_TRACE )
                v.emplace_back( reader.first );
    }

    return v;
}

bool
mzML::applyCalibration( const adcontrols::MSCalibrateResult& calibResult )
{
    return adutils::mscalibio::write( dbf_.db(), calibResult );
}

bool
mzML::loadCalibrations()
{
    // nothing to be done
    return false;
}

std::shared_ptr< adcontrols::MassSpectrometer >
mzML::getSpectrometer( uint64_t objid, const std::wstring& dataInterpreterClsid ) const
{
    // nothing to be done (this was for v2 data)
	assert(0);
	return nullptr;
    //return const_cast<mzML *>(this)->getSpectrometer( objid, dataInterpreterClsid );
}

size_t
mzML::getSpectrumCount( int fcn ) const
{
    ADDEBUG() << "################## " << __FUNCTION__ << " ##";
    int tfcn(0);
    if ( auto reader = findDataReader( fcn, tfcn ) ) {
        if ( auto tic = reader->TIC( tfcn ) )
            return tic->size();
    }
    return 0;
}

bool
mzML::getSpectrum( int fcn, size_t pos, adcontrols::MassSpectrum& ms, uint32_t objid ) const
{
    assert( 0 );
    return true;
}

bool
mzML::index( size_t pos, int& idx, int& fcn, int& rep, double * time ) const
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
mzML::find_scan( int idx, int fcn ) const
{
    return size_t(-1);
}

int
mzML::make_index( size_t pos, int& fcn ) const
{
    return 0;
}

bool
mzML::getTIC( int fcn, adcontrols::Chromatogram& c ) const
{
    ADDEBUG() << "################### " << __FUNCTION__ << " ##";
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
mzML::getChromatogramCount() const
{
    return 0;
}

size_t
mzML::getFunctionCount() const
{
    return fcnCount_;
}

size_t
mzML::posFromTime( double seconds ) const
{
    ADDEBUG() << "################### " << __FUNCTION__ << " ##";
    for ( auto& reader : readers_ ) {
        if ( auto tpos = reader.first->findPos( seconds ) )
            return tpos->time_since_inject();
    }
	return 0;
}

double
mzML::timeFromPos( size_t pos ) const
{
    ADDEBUG() << "################### " << __FUNCTION__ << " ##";
    if ( !times_.empty() && pos < times_.size() )
        return times_[ pos ].first;
	return 0;
}

bool
mzML::getChromatograms( const std::vector< std::tuple<int, double, double> >& ranges
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
mzML::fetchTraces( int64_t objid, const adcontrols::DataInterpreter& interpreter, adcontrols::TraceAccessor& accessor )
{
    ADDEBUG() << "## " << __FUNCTION__ << " ##";
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
mzML::fetchSpectrum( int64_t objid
                        , const std::wstring& dataInterpreterClsid
                        , uint64_t npos, adcontrols::MassSpectrum& ms
						, const std::wstring& traceId ) const
{
    // this method no longer supported for v3
    assert( 0 );
    return adcontrols::no_interpreter;
}

bool
mzML::hasProcessedSpectrum( int, int ) const
{
    // this method no longer supported for v3
    assert( 0 );
    return false;
}

uint32_t
mzML::findObjId( const std::wstring& traceId ) const
{
    // this method no longer supported for v3
    assert( 0 );
    return false;
}

bool
mzML::getRaw( uint64_t objid, uint64_t npos, uint64_t& fcn, std::vector< char >& xdata, std::vector< char >& xmeta ) const
{
    // this method no longer supported for v3
    assert( 0 );
    return false;
}

bool
mzML::mslocker( adcontrols::lockmass::mslock &mslk, uint32_t objid ) const
{
    // this method no longer supported for v3
    assert( 0 );
    return false;
}

std::shared_ptr< adcontrols::DataReader >
mzML::findDataReader( int fcn, int& xfcn ) const
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
#endif
