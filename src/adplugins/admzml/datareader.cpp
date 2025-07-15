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

#include "datareader.hpp"
#include "accession.hpp"
#include "mzmlspectrum.hpp"
#include "chromatogram.hpp"
#include "scan_protocol.hpp"
#include "mzml.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adportable/debug.hpp>
#include <boost/json.hpp>
#include <boost/format.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <stdexcept>

namespace mzml {
    namespace local {

        class data_reader::impl {
        public:
            impl( std::shared_ptr< const mzML > mzml
                  , int fcn ) : mzml_( mzml )
                              , fcn_( fcn ) {
                objtext_ = (boost::format("%1%.admzml.ms-cheminfo.com") % fcn_).str();
            }
            std::shared_ptr< const mzML > mzml_;
            int fcn_;
            std::string objtext_;
            mzml::scan_protocol scan_protocol_;
            mzml::scan_protocol_key_t protocol_key_;
            std::string traceid_;

            // 57B0153C-6BDD-41AF-82E9-6C32772841A0
            constexpr const static boost::uuids::uuid uuid_ = {
                0x57, 0xB0, 0x15, 0x3C, 0x6B, 0xDD, 0x41, 0xAF
                , 0x82, 0xE9, 0x6C, 0x32, 0x77, 0x28, 0x41, 0xA0 };

            std::string display_name_;
        };
    }
}

using namespace mzml::local;

data_reader::~data_reader()
{
}

data_reader::data_reader( const char * traceid
                          , int fcn
                          , std::shared_ptr< const mzML > mzml )
    : adcontrols::DataReader( traceid )
    , impl_( std::make_unique< impl >( mzml, fcn ) )
{
    auto jv = boost::json::parse( traceid );
    impl_->traceid_ = boost::json::serialize( jv );
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


const boost::uuids::uuid&
data_reader::__uuid__()
{
    return impl::uuid_;
}

const std::string&
data_reader::traceid() const
{
    return impl_->traceid_;
}

std::string
data_reader::abbreviated_display_name() const
{
    return impl_->display_name_;
}

bool
data_reader::initialize( std::shared_ptr< adfs::sqlite > db, const boost::uuids::uuid& objid, const std::string& objtext )
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    return true;
}

void
data_reader::finalize()
{
}

const boost::uuids::uuid&
data_reader::objuuid() const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ================= " << impl_->uuid_;
    return impl_->uuid_;
}

const std::string&
data_reader::objtext() const
{
    return impl_->objtext_;
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
    return std::accumulate( impl_->mzml_->scan_indices().begin()
                            , impl_->mzml_->scan_indices().end()
                            , 0
                            , [&]( const auto& a, const auto& b ){
                                return (b.second->protocol_id() == impl_->fcn_ ? 1 : 0) + a;
                            });
    // return impl_->mzml_->getSpectrumCount( fcn );
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
    if ( auto key = impl_->mzml_->find_key_by_index( impl_->fcn_ ) ) {
        auto& indices = impl_->mzml_->scan_indices();

        auto it = std::find_if( indices.begin()
                                , indices.end()
                                , [&](const auto& a){
                                    return std::get< enum_scan_protocol >(a.first).protocol_key() == *key &&
                                        (std::get<enum_scan_start_time>(a.first) >= seconds);
                                });

        if ( it != indices.end() ) {
            size_t rowid = std::distance( indices.begin(), it );
            if ( closest && (it+1) != indices.end() ) {
                if ( std::abs( std::get< enum_scan_start_time >(it->first) - seconds )
                     > std::abs( std::get< enum_scan_start_time >((it + 1)->first) - seconds ) )
                    ++rowid;
            }
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
    auto& indices = impl_->mzml_->scan_indices();
    if ( 0 <= pos && pos <= indices.size() )
        return std::get< enum_scan_start_time >( indices[ pos ].first );
    return -1.0;
}

std::shared_ptr< const adcontrols::Chromatogram >
data_reader::TIC( int fcn ) const
{
    if ( auto chro = std::shared_ptr< adcontrols::Chromatogram >() ) {
        impl_->mzml_->getTIC( fcn, *chro );
        return chro;
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
    if ( (rowid + 1) < impl_->mzml_->scan_indices().size() ) {
        auto it = impl_->mzml_->scan_indices().begin() + rowid + 1;
        while ( it != impl_->mzml_->scan_indices().end() ) {
            if ( it->second->protocol_id() == fcn )
                return std::distance( impl_->mzml_->scan_indices().begin(), it );
            ++it;
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
    while ( --rowid >= 0 ) {
        auto it = impl_->mzml_->scan_indices().begin() + rowid;
        if ( it->second->protocol_id() == fcn )
            return std::distance( impl_->mzml_->scan_indices().begin(), it );
    }
    return 0;
}

int64_t
data_reader::pos( int64_t rowid ) const
{
    // convert rowid --> pos, a.k.a. trigger number since injected.
    return rowid;
}

int64_t
data_reader::elapsed_time( int64_t rowid ) const
{
    if ( 0 <= rowid && rowid < impl_->mzml_->scan_indices().size() ) {
        auto it = impl_->mzml_->scan_indices().begin() + rowid;
        return std::get< enum_scan_start_time >( it->first );
    }
    return -1;
}

int64_t
data_reader::epoch_time( int64_t rowid ) const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ================== rowid = " << rowid;
    return -1;
}

double
data_reader::time_since_inject( int64_t rowid ) const
{
    if ( 0 <= rowid && rowid < impl_->mzml_->scan_indices().size() ) {
        auto it = impl_->mzml_->scan_indices().begin() + rowid;
        return std::get< enum_scan_start_time >( it->first );
    }
    return -1;
}

int
data_reader::fcn( int64_t rowid ) const
{
    if ( 0 <= rowid && rowid < impl_->mzml_->scan_indices().size() ) {
        auto it = impl_->mzml_->scan_indices().begin() + rowid;
        return it->second->protocol_id();
    }
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
    if ( it->rowid() < impl_->mzml_->scan_indices().size() ) {
        const auto& datum = impl_->mzml_->scan_indices()[ it->rowid() ];
        return datum.second->toMassSpectrum( *datum.second );
    }
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " =============== return nullptr";
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
    auto it = impl_->mzml_->scan_indices().begin() + first.rowid();
    if ( it->second->protocol_id() != impl_->fcn_ )
        throw std::invalid_argument("protocol id missmatch");

    auto ms = it->second->toMassSpectrum( *it->second );
    std::vector< double > intensities ( ms->size(), 0 );
    last++; // advance for pointing end

    for ( auto it = first; it != last; it++ ) {
        const auto sp = impl_->mzml_->scan_indices()[ it.rowid() ].second;
        if ( sp->protocol_id() != impl_->fcn_ ) {
            ADDEBUG() << "Internal error -- protocol id missmatch " << std::format("{} != {}", sp->protocol_id(), impl_->fcn_);
            continue;
        }
        const auto& secondi = sp->dataArrays().second;
        std::visit([&](auto arg){
            for ( size_t i = 0; i < secondi.length(); ++i )
                intensities[i] += *arg++;
        }, secondi.data() );
    }
    ms->setIntensityArray( std::move( intensities ) );
    return ms;
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
