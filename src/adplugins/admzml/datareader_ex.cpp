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
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
#include <adportable/bzip2.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <algorithm>
#include <numeric>
#include <memory>
#include <sstream>
#include <stdexcept>

namespace mzml {
    namespace exposed {

        template< typename Interpreter, int = 0 > struct TID {
            static const std::string value;
            static const std::string display_name;
            typedef Interpreter type;
        };

        // template<> const std::string TID< spectrum::DataInterpreter< mzml::spectrum > >::value = "1.mzml.ms-cheminfo.com";
        enum scan_index_enum {
            scan_index_rowid
            , scan_index_elapsed_time
            , scan_index_pos
            , scan_index_fcn
            , scan_index_scan_id
            , scan_index_spectrum };

        class data_reader::impl {
        public:
            impl() : fcn_( 0 ) {
            }
            std::weak_ptr< adfs::sqlite > db_;
            int fcn_;
            mzml::scan_protocol scan_protocol_;
            mzml::scan_protocol_key_t protocol_key_;
            using index_t = std::tuple< int64_t    // rowid
                                        , double   // elapsed_time
                                        , int32_t  // pos
                                        , int32_t  // fcn
                                        , mzml::scan_id
                                        , std::shared_ptr<mzml::mzMLSpectrum> >;
            std::vector< index_t > scan_indices_;
            std::vector< index_t >::const_iterator find_rowid( int64_t rowid ) const {
                return std::lower_bound( scan_indices_.begin()
                                         , scan_indices_.end()
                                         , rowid
                                         , []( const auto& a, int64_t b ){
                                             return std::get<0>(a) < b;
                                         });
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
                auto elapsed_time = sql.get_column_value< int64_t >( 1 ) * 1.0e-9;
                auto pos = sql.get_column_value< int64_t >( 2 );
                auto fcn = sql.get_column_value< int64_t >( 3 );
                auto events = sql.get_column_value< int64_t >( 4 );
                auto xdata = sql.get_column_value< adfs::blob >( 5 );

                std::string inflated;
                adportable::bzip2::decompress( inflated, reinterpret_cast<const char *>(xdata.data()), xdata.size() );
                if ( auto spc = serializer::deserialize( inflated.data(), inflated.size() ) ) {
                    spc->set_protocol_id( fcn );
                    impl_->scan_indices_.emplace_back( rowid, elapsed_time, pos, fcn, spc->get_scan_id(), spc );
                    ADDEBUG() << std::make_tuple( rowid, elapsed_time, pos, fcn ) << boost::json::value_from( spc->get_scan_id() );
                } else {
                    ADDEBUG() << "--------- deerialize failed -----------";
                }
            } catch ( const boost::exception& ex ) {
                ADDEBUG() << "Exception: " << ex;
            }
        }
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
    if ( auto db = impl_->db_.lock() ) {
        adfs::stmt sql( *db );
        sql.prepare( "SELECT COUNT(*) FROM AcquiredData WHERE objuuid=? AND fcn=?" );
        sql.bind( 1 ) = impl::uuid_;
        sql.bind( 2 ) = fcn;
        if ( sql.step() == adfs::sqlite_done ) {
            return sql.get_column_value< int64_t >( 0 );
        }
    }
    return 0;
}

adcontrols::DataReader::const_iterator
data_reader::begin( int fcn ) const
{
    return adcontrols::DataReader_iterator( this, 0, impl_->fcn_ );
}

adcontrols::DataReader::const_iterator
data_reader::end() const
{
    return adcontrols::DataReader_iterator( this, (-1) );
}

adcontrols::DataReader::const_iterator
data_reader::findPos( double seconds, int fcn, bool closest, TimeSpec tspec ) const
{
    if ( auto db = impl_->db_.lock() ) {
        adfs::stmt sql( *db );
        sql.prepare( "SELECT rowid FROM AcquiredData WHERE objuuid=? AND fcn=? AND elapsed_time ORDER BY ABS(? - elapsed_time) LIMIT 1" );
        sql.bind(1) = impl::uuid_;
        sql.bind(2) = impl_->fcn_;
        sql.bind(3) = seconds * 1e9;
        if ( sql.step() == adfs::sqlite_done ) {
            auto rowid = sql.get_column_value< int64_t >( 0 );
            return adcontrols::DataReader_iterator( this, rowid, impl_->fcn_ );
        }
    }
    // if ( auto key = impl_->mzml_->find_key_by_index( impl_->fcn_ ) ) {
    //     auto& indices = impl_->mzml_->scan_indices();

    //     auto it = std::find_if( indices.begin()
    //                             , indices.end()
    //                             , [&](const auto& a){
    //                                 return std::get< enum_scan_protocol >(a.first).protocol_key() == *key &&
    //                                     (std::get<enum_scan_start_time>(a.first) >= seconds);
    //                             });

    //     if ( it != indices.end() ) {
    //         size_t rowid = std::distance( indices.begin(), it );
    //         if ( closest && (it+1) != indices.end() ) {
    //             if ( std::abs( std::get< enum_scan_start_time >(it->first) - seconds )
    //                  > std::abs( std::get< enum_scan_start_time >((it + 1)->first) - seconds ) )
    //                 ++rowid;
    //         }
    //         return adcontrols::DataReader_iterator( this, rowid, impl_->fcn_ );
    //     }
    // }
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
    if ( it != impl_->scan_indices_.end() && std::get<0>(*it) == rowid )
        return std::get< scan_index_elapsed_time >( *it );
    return -1;
}

int64_t
data_reader::epoch_time( int64_t rowid ) const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ================== rowid = " << rowid;
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
    auto it = std::lower_bound( impl_->scan_indices_.begin()
                                , impl_->scan_indices_.end()
                                , rowid
                                , []( const auto& a, int64_t b ){
                                    return std::get<0>(a) < b;
                                });
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
    auto It = impl_->find_rowid( it->rowid() );
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
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    auto it0 = impl_->find_rowid( first.rowid() );
    auto it1 = impl_->find_rowid( last.rowid() );
    if ( it0 != impl_->scan_indices_.end() && it1 != impl_->scan_indices_.end() ) {
        ++it0; // advance to pointing end
        auto spc = std::get< scan_index_spectrum >( *it0 );
        if ( spc->protocol_id() != impl_->fcn_ )
            throw std::invalid_argument("protocol id missmatch");
        auto ms = mzMLSpectrum::toMassSpectrum( *std::get< scan_index_spectrum >( *it0 ) );
        std::vector< double > intensities ( ms->size(), 0 );
        for ( auto it = it0; it != it1; ++it ) {
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
