/**************************************************************************
 ** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
 ** Copyright (C) 2013-2025 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "datareader_ex.hpp"
#include "accession.hpp"
#include "mzmlspectrum.hpp"
#include "chromatogram.hpp"
#include "scan_protocol.hpp"
#include "mzml.hpp"
#include "mzmlreader.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
#include <adportable/bzip2.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <ratio>

namespace {
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    // template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
}

namespace mzml {
    namespace exposed {

        template< typename Interpreter, int = 0 > struct TID {
            static const std::string value;
            static const std::string display_name;
            typedef Interpreter type;
        };

        // template<> const std::string TID< spectrum::DataInterpreter< mzml::spectrum > >::value = "1.mzml.ms-cheminfo.com";
        enum scan_index_enum {
            scan_index_rowid           // 0
            , scan_index_elapsed_time  // 1
            , scan_index_pos           // 2
            , scan_index_fcn           // 3
            , scan_index_scan_id       // 4
            , scan_index_spectrum };   // 5

        class data_reader::impl {
        public:
            impl() : fcn_( 0 ) {
            }
            std::weak_ptr< adfs::sqlite > db_;
            int fcn_;
            mzml::scan_protocol scan_protocol_;
            mzml::scan_protocol_key_t protocol_key_;
            using index_t = std::tuple< int64_t     // 0: rowid
                                        , int64_t   // 1: elapsed_time (ns)
                                        , int32_t   // 2: pos
                                        , int32_t   // 3: fcn
                                        , mzml::scan_id // 4:
                                        , std::shared_ptr<mzml::mzMLSpectrum> >; // 5

            std::vector< index_t > scan_indices_;
            std::vector< std::shared_ptr< pugi::xml_document > > xml_document_holder_;

            std::vector< index_t >::const_iterator find_index( int64_t rowid ) const {
                return std::lower_bound( scan_indices_.begin()
                                            , scan_indices_.end()
                                            , rowid
                                            , []( const auto& a, int64_t b ){
                                                return std::get<0>(a) < b;
                                            });
            }

            std::pair< std::vector< index_t >::const_iterator
                       , std::vector< index_t >::const_iterator >
            find_index_range_forward( const adcontrols::DataReader::const_iterator& first
                                      , const adcontrols::DataReader::const_iterator& last ) const {
                std::pair< std::vector< index_t >::const_iterator
                           , std::vector< index_t >::const_iterator >
                    v( find_index( first->rowid() ), find_index(last->rowid() ) );
                if ( v.second != scan_indices_.end() )
                    v.second++;
                return v;
            }

            // 6a6cf573-ef05-4c5c-a607-0a417edf37b0
            constexpr const static boost::uuids::uuid uuid_ = {
                0x6a, 0x6c, 0xf5, 0x73, 0xef, 0x05, 0x4c, 0x5c
                , 0xa6, 0x07, 0x0a, 0x41, 0x7e, 0xdf, 0x37, 0xb0 };
            std::string display_name_;
            const static std::string objtext__;
        };

        std::string const data_reader::impl::objtext__ = "1.admzml.ms-cheminfo.com";
    }
}

using namespace mzml::exposed;


data_reader::~data_reader()
{
}

data_reader::data_reader( const char * traceid ) : adcontrols::DataReader( traceid )
                                                 , impl_( std::make_unique< impl >() )
{
    auto jv = boost::json::parse( traceid );
    ADDEBUG() << __FUNCTION__ << "traceid: " << jv;

    impl_->scan_protocol_ = boost::json::value_to< scan_protocol >( jv );
    impl_->protocol_key_ = impl_->scan_protocol_.protocol_key();

    std::ostringstream o;
    o << boost::format("MS%d") % impl_->scan_protocol_.ms_level();
    if ( impl_->scan_protocol_.ms_level() == 2 ) { // pscan
        o << boost::format( " %.1f[%.1fV] " )
            % impl_->scan_protocol_.precursor_mz()
            % impl_->scan_protocol_.collision_energy();
    }
    o << (impl_->scan_protocol_.polarity() == polarity_negative ? "(−)"
          : impl_->scan_protocol_.polarity() == polarity_positive ? "(+)"
          : "(?)");
    impl_->display_name_ = o.str();
}

std::string
data_reader::abbreviated_display_name() const
{
    return impl_->display_name_;
}

// entry for addatafile::datafile
bool
data_reader::initialize( std::shared_ptr< adfs::sqlite > dbf, const boost::uuids::uuid& objid, const std::string& objtext )
{
    impl_->db_ = dbf;

    if ( auto db = impl_->db_.lock() ) {
        adfs::stmt sql( *db );
        sql.prepare( "SELECT rowid,elapsed_time,npos,fcn,events,data FROM AcquiredData WHERE objuuid=?" );
        sql.bind(1) = impl::uuid_;
        while ( sql.step() == adfs::sqlite_row ) {
            try {
                auto rowid = sql.get_column_value< int64_t >( 0 );
                auto elapsed_time = sql.get_column_value< int64_t >( 1 ); // ns
                auto pos = sql.get_column_value< int64_t >( 2 );
                auto fcn = sql.get_column_value< int64_t >( 3 );
                auto events = sql.get_column_value< int64_t >( 4 );
                auto xdata = sql.get_column_value< adfs::blob >( 5 );

                if ( auto doc = serializer::deserialize( reinterpret_cast< const char * >(xdata.data()), xdata.size() ) ) {
                    impl_->xml_document_holder_.emplace_back( doc );
                    if ( auto node = doc->select_node( "spectrum" ) ) {
                        auto var = mzMLReader{}( node.node() );
                        auto spc = std::visit( overloaded{
                                []( std::shared_ptr< mzMLChromatogram > )->std::shared_ptr< mzMLSpectrum >{ return nullptr; }
                                    , []( std::shared_ptr< mzMLSpectrum> t ) { return t; }
                                    }, var );
                        if ( spc ) {
                            spc->set_protocol_id( fcn );
                            impl_->scan_indices_.emplace_back( rowid, elapsed_time, pos, fcn, spc->get_scan_id(), spc );
                        }
                    }

                } else {
                    ADDEBUG() << "--------- deerialize failed -----------";
                }
            } catch ( const boost::exception& ex ) {
                ADDEBUG() << "Exception: " << ex;
            }
        }
        ADDEBUG() << "scan_indices.size() = " << impl_->scan_indices_.size();
        size_t i=0;
        for ( const auto& idx: impl_->scan_indices_ )
            ADDEBUG() << i++ << ", " << std::get<0>( idx )
                      << ", " << std::get<1>( idx )
                      << ", " <<  std::get<3>( idx )
                      << ", " << std::get< scan_index_spectrum >( idx ).get()
                      << ", " << ( std::get< scan_index_spectrum >( idx ) ? std::get< scan_index_spectrum >( idx )->length() : 0 )
                ;

    }
    return true;
}

void
data_reader::finalize()
{
    ADDEBUG() << "## " << __FUNCTION__;
}

//static
const boost::uuids::uuid&
data_reader::__objuuid__()
{
    return impl::uuid_;
}

//static
const std::string&
data_reader::__objtext__()
{
    return impl::objtext__;
}


const boost::uuids::uuid&
data_reader::objuuid() const
{
    return impl_->uuid_;
}

const std::string&
data_reader::objtext() const
{
    return impl::objtext__;
}

int64_t
data_reader::objrowid() const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    return 0;
}

const std::string&
data_reader::display_name() const
{
    return impl_->display_name_;
}

size_t
data_reader::fcnCount() const
{
    return 1; //impl_->mzml_->getFunctionCount();
}

size_t
data_reader::size( int fcn /* segmented fcn */ ) const
{
    ADDEBUG() << "### " << __FUNCTION__ << " ### " << fcn;

    if ( auto db = impl_->db_.lock() ) {
        adfs::stmt sql( *db );
        if ( fcn < 0 ) {
            sql.prepare( "SELECT COUNT(*) FROM AcquiredData WHERE objuuid=?" );
            sql.bind( 1 ) = impl::uuid_;
        } else {
            sql.prepare( "SELECT COUNT(*) FROM AcquiredData WHERE objuuid=? AND fcn=?" );
            sql.bind( 1 ) = impl::uuid_;
            sql.bind( 2 ) = fcn;
        }
        if ( sql.step() == adfs::sqlite_done ) {
            return sql.get_column_value< int64_t >( 0 );
        }
    }
    return 0;
}

adcontrols::DataReader::const_iterator
data_reader::begin( int fcn ) const
{
    ADDEBUG() << "### " << __FUNCTION__ << " ### " << fcn;

    if ( auto db = impl_->db_.lock() ) {
        adfs::stmt sql( *db );
        if ( fcn >= 0 ) {
            sql.prepare( "SELECT MIN(rowid) FROM AcquiredData WHERE objuuid=? AND fcn=?" );
            sql.bind(1) = impl::uuid_;
            sql.bind(2) = impl_->fcn_;
        } else {
            sql.prepare( "SELECT MIN(rowid) FROM AcquiredData WHERE objuuid=?" );
            sql.bind(1) = impl::uuid_;
        }
        if ( sql.step() == adfs::sqlite_done ) {
            auto rowid = sql.get_column_value< int64_t >( 0 );
            return adcontrols::DataReader_iterator( this, rowid, impl_->fcn_ );
        }
    }
    if ( not impl_->scan_indices_.empty() )
        return adcontrols::DataReader_iterator( this, std::get<0>(impl_->scan_indices_.front()), impl_->fcn_ );
    return end();
}

adcontrols::DataReader::const_iterator
data_reader::end() const
{
    return adcontrols::DataReader_iterator( this, (-1) );
}

adcontrols::DataReader::const_iterator
data_reader::findPos( double seconds, int fcn, bool closest, TimeSpec tspec ) const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ================== " << std::make_tuple( seconds, fcn, closest );

    if ( auto db = impl_->db_.lock() ) {
        adfs::stmt sql( *db );
        if ( fcn >= 0 ) {
            sql.prepare( "SELECT rowid FROM AcquiredData WHERE objuuid=? AND fcn=? AND elapsed_time ORDER BY ABS(? - elapsed_time) LIMIT 1" );
            sql.bind(1) = impl::uuid_;
            sql.bind(2) = impl_->fcn_;
            sql.bind(3) = seconds * 1e9;
        } else {
            sql.prepare( "SELECT rowid FROM AcquiredData WHERE objuuid=? AND elapsed_time ORDER BY ABS(? - elapsed_time) LIMIT 1" );
            sql.bind(1) = impl::uuid_;
            sql.bind(2) = seconds * 1e9;
        }
        if ( sql.step() <= adfs::sqlite_row ) {
            auto rowid = sql.get_column_value< int64_t >( 0 );
            ADDEBUG() << sql.expanded_sql();
            ADDEBUG() << "\t---> rowid: " << std::make_pair(rowid, fcn );
            return adcontrols::DataReader_iterator( this, rowid, impl_->fcn_ );
        }
    }
    return end();
}

double
data_reader::findTime( int64_t pos, IndexSpec ispec, bool exactMatch ) const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";

    assert( ispec == TriggerNumber );
    auto& indices = impl_->scan_indices_;
    if ( 0 <= pos && pos <= indices.size() )
        return std::get< 1 >(indices[ pos ] );
    return -1.0;
}

std::shared_ptr< const adcontrols::Chromatogram >
data_reader::TIC( int fcn ) const
{
    if ( auto chro = std::shared_ptr< adcontrols::Chromatogram >() ) {
        // impl_->mzml_->getTIC( fcn, *chro );
        // return chro;
    }
    return nullptr;
}

int64_t
data_reader::next( int64_t rowid ) const
{
    return next( rowid, impl_->fcn_ );
}

int64_t
data_reader::next( int64_t rowid, int fcn ) const
{
    if ( !impl_->scan_indices_.empty() ) {
        auto it = std::lower_bound( impl_->scan_indices_.begin()
                                    , impl_->scan_indices_.end()
                                    , rowid
                                    , []( const auto& a, int64_t b ){
                                        return std::get<0>(a) < b;
                                    });
        if ( it != impl_->scan_indices_.end() ) {
            if ( ++it != impl_->scan_indices_.end() ) {
                return std::get<0>(*it);
            }
        }
    } else {
        if ( auto db = impl_->db_.lock() ) {
            adfs::stmt sql( *db );
            sql.prepare( "SELECT rowid FROM AcquiredData WHERE objuuid = ? AND fcn = ? AND npos > (SELECT npos FROM AcquiredData WHERE rowid=?) LIMIT 1" );
            sql.bind( 1 ) = impl::uuid_;
            sql.bind( 2 ) = impl_->fcn_;
            sql.bind( 3 ) = rowid;
            if ( sql.step() == adfs::sqlite_row )
                return sql.get_column_value< int64_t >( 0 );
        }
    }
    return (-1);
}

int64_t
data_reader::prev( int64_t rowid ) const
{
    return prev( rowid, impl_->fcn_ );
}

int64_t
data_reader::prev( int64_t rowid, int fcn ) const
{
    if ( !impl_->scan_indices_.empty() ) {
        auto it = std::lower_bound( impl_->scan_indices_.begin()
                                    , impl_->scan_indices_.end()
                                    , rowid
                                    , []( const auto& a, int64_t b ){
                                        return std::get<0>(a) < b;
                                    });
        if ( it != impl_->scan_indices_.begin() ) {
            it--;
            std::get<0>(*it);
        }
    }
    return 0;
}

int64_t
data_reader::pos( int64_t rowid ) const
{
    // convert rowid --> pos, a.k.a. trigger number since injected.
    auto it = std::lower_bound( impl_->scan_indices_.begin()
                                , impl_->scan_indices_.end()
                                , rowid
                                , []( const auto& a, int64_t b ){
                                    return std::get<0>(a) < b;
                                });
    if ( it != impl_->scan_indices_.end() && std::get<0>(*it) == rowid )
        return std::get< scan_index_pos >( *it );
    return 0;
}

int64_t
data_reader::elapsed_time( int64_t rowid ) const
{
    auto it = std::lower_bound( impl_->scan_indices_.begin()
                                , impl_->scan_indices_.end()
                                , rowid
                                , []( const auto& a, int64_t b ){
                                    return std::get<0>(a) < b;
                                });
    // ADDEBUG() << "## " << __FUNCTION__ << " ## rowid=" << rowid << "\t" << *it;
    if ( it != impl_->scan_indices_.end() && std::get<0>(*it) == rowid )
        return std::get< scan_index_elapsed_time >( *it ); // ns
    return -1;
}

int64_t
data_reader::epoch_time( int64_t rowid ) const
{
    // Shimadzu mzML file does not provide injection time point
    return elapsed_time( rowid );
}

double
data_reader::time_since_inject( int64_t rowid ) const
{
    return elapsed_time( rowid );
}

int
data_reader::fcn( int64_t rowid ) const
{
    auto it = impl_->find_index( rowid );
    if ( it != impl_->scan_indices_.end() && std::get<0>(*it) == rowid )
        return std::get< scan_index_fcn >( *it );
    return -1;
}

boost::any
data_reader::getData( int64_t rowid ) const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    return {};
}

std::shared_ptr< adcontrols::MassSpectrum >
data_reader::getSpectrum( int64_t rowid ) const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    return nullptr;
}

std::shared_ptr< adcontrols::MassSpectrum >
data_reader::readSpectrum( const const_iterator& it ) const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    auto It = impl_->find_index( it->rowid() );
    if ( It != impl_->scan_indices_.end() && std::get<0>(*It) == it->rowid() ) {
        auto spc = std::get< scan_index_spectrum >( *It );
        return spc->toMassSpectrum( *spc );
    }
    return {};
}

std::shared_ptr< adcontrols::Chromatogram >
data_reader::getChromatogram( int fcn, double time, double width ) const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    return nullptr;
}

std::shared_ptr< adcontrols::MassSpectrum >
data_reader::coaddSpectrum( const_iterator&& first, const_iterator&& last ) const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ====== " << std::make_pair(first->elapsed_time()*1e-9 , last->elapsed_time()*1e-9 );

    auto [beg,end] = impl_->find_index_range_forward( first, last );

    if ( beg != impl_->scan_indices_.end() ) {

        if ( auto sp = std::get< scan_index_spectrum >( *beg ) ) {
            if ( sp->protocol_id() != impl_->fcn_ )
                throw std::invalid_argument("protocol id missmatch");

            ADDEBUG() << "first index: " << *beg << ", protocol_id: " << sp->protocol_id() << " == fcn_: " << impl_->fcn_;
            ADDEBUG() << "sp= " << sp.get() << ", length: " << sp->length();

            auto ms = mzMLSpectrum::toMassSpectrum( *sp );
            ADDEBUG() << "ms = " << ms.get();
            std::vector< double > intensities ( ms->size(), 0 );
            ADDEBUG() << "ms.size: " << ms->size();

            for ( auto it = beg; it != end; ++it ) {
                ADDEBUG() << "rowid: " << std::get<0>(*it) << ", " << std::get< scan_index_elapsed_time >(*it) / 1e9;

                if ( std::get< scan_index_fcn >(*it) == impl_->fcn_ ) {
                    auto sp = std::get< scan_index_spectrum >( *it );
                    const auto secondi = sp->dataArrays().second;
                    std::visit([&](auto arg){
                        for ( size_t i = 0; i < secondi.length(); ++i )
                            intensities[i] += *arg++;
                    }, secondi.data() );
                }
            }
            ms->setIntensityArray( std::move( intensities ) );
            return ms;
        }

    }
    return {};
}

std::shared_ptr< adcontrols::MassSpectrometer >
data_reader::massSpectrometer() const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    return {}; // spectrometer_;
}

adcontrols::DataInterpreter *
data_reader::dataInterpreter() const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    return {}; // interpreter_.get();
}

void
data_reader::handleCalibrateResultAltered() const
{
}
