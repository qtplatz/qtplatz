/**************************************************************************
 ** Copyright (C) 2010-2024 Toshinobu Hondo, Ph.D.
 ** Copyright (C) 2013-2024 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <algorithm>
#include <numeric>
#include <sstream>
#include <stdexcept>

namespace mzml {

    class DataReader::impl {
    public:
        impl( std::shared_ptr< const mzML > mzml
              , int fcn ) : mzml_( mzml )
                        , fcn_( fcn ) {
        }
        std::shared_ptr< const mzML > mzml_;
        int fcn_;
        mzml::scan_protocol scan_protocol_;
        mzml::scan_protocol_key_t protocol_key_;

        // 6a6cf573-ef05-4c5c-a607-0a417edf37b0
        constexpr const static boost::uuids::uuid uuid_ = {
            0x6a, 0x6c, 0xf5, 0x73, 0xef, 0x05, 0x4c, 0x5c
            , 0xa6, 0x07, 0x0a, 0x41, 0x7e, 0xdf, 0x37, 0xb0 };

        static const std::string objtext_;
        std::string display_name_;
    };

    const std::string DataReader::impl::objtext_ = "1.admzml.ms-cheminfo.com";
    // const std::string DataReader::impl::display_name_ = "mzML";
}

using namespace mzml;


DataReader::~DataReader()
{
}

DataReader::DataReader( const char * traceid
                        , int fcn
                        , std::shared_ptr< const mzML > mzml ) : adcontrols::DataReader( traceid )
                                                               , impl_( std::make_unique< impl >( mzml, fcn ) )
{
    auto jv = boost::json::parse( traceid );
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
    ADDEBUG() << "display_name: " << impl_->display_name_;
}

std::string
DataReader::abbreviated_display_name() const
{
    return impl_->display_name_;
}

bool
DataReader::initialize( adfs::filesystem& dbf, const boost::uuids::uuid& objid, const std::string& objtext )
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    return true;
}

void
DataReader::finalize()
{
}

const boost::uuids::uuid&
DataReader::objuuid() const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    return impl_->uuid_;
}

const std::string&
DataReader::objtext() const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    return impl_->objtext_;
}

int64_t
DataReader::objrowid() const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    return 0;
}

const std::string&
DataReader::display_name() const
{
    return impl_->display_name_;
}

size_t
DataReader::fcnCount() const
{
    return 1; //impl_->mzml_->getFunctionCount();
}

size_t
DataReader::size( int fcn ) const
{
    return impl_->mzml_->getSpectrumCount( fcn );
}

adcontrols::DataReader::const_iterator
DataReader::begin( int fcn ) const
{
    return adcontrols::DataReader_iterator( this, 0, fcn );
}

adcontrols::DataReader::const_iterator
DataReader::end() const
{
    return adcontrols::DataReader_iterator( this, (-1) );
}

adcontrols::DataReader::const_iterator
DataReader::findPos( double seconds, int fcn, bool closest, TimeSpec tspec ) const
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
            ADDEBUG() << "\tfound rowid=" << rowid << ", key=" << std::get< enum_scan_protocol >(it->first).protocol_key() << ", *key=" << *key
                      << " --> " << (std::get< enum_scan_protocol >(it->first).protocol_key() == *key);

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
DataReader::findTime( int64_t pos, IndexSpec ispec, bool exactMatch ) const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";

    assert( ispec == TriggerNumber );
    auto& indices = impl_->mzml_->scan_indices();
    if ( 0 <= pos && pos <= indices.size() )
        return std::get< enum_scan_start_time >( indices[ pos ].first );
    return -1.0;
}

std::shared_ptr< const adcontrols::Chromatogram >
DataReader::TIC( int fcn ) const
{
    if ( auto chro = std::shared_ptr< adcontrols::Chromatogram >() ) {
        impl_->mzml_->getTIC( fcn, *chro );
        return chro;
    }
    return nullptr;
}

int64_t
DataReader::next( int64_t rowid ) const
{
    return next( rowid, impl_->fcn_ );
}

int64_t
DataReader::next( int64_t rowid, int fcn ) const
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
DataReader::prev( int64_t rowid ) const
{
    return prev( rowid, impl_->fcn_ );
}

int64_t
DataReader::prev( int64_t rowid, int fcn ) const
{
    while ( --rowid >= 0 ) {
        auto it = impl_->mzml_->scan_indices().begin() + rowid;
        if ( it->second->protocol_id() == fcn )
            return std::distance( impl_->mzml_->scan_indices().begin(), it );
    }
    return 0;
}

int64_t
DataReader::pos( int64_t rowid ) const
{
    // convert rowid --> pos, a.k.a. trigger number since injected.
    return rowid;
}

int64_t
DataReader::elapsed_time( int64_t rowid ) const
{
    if ( 0 <= rowid && rowid < impl_->mzml_->scan_indices().size() ) {
        auto it = impl_->mzml_->scan_indices().begin() + rowid;
        return std::get< enum_scan_start_time >( it->first );
    }
    return -1;
}

int64_t
DataReader::epoch_time( int64_t rowid ) const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ================== rowid = " << rowid;
    return -1;
}

double
DataReader::time_since_inject( int64_t rowid ) const
{
    if ( 0 <= rowid && rowid < impl_->mzml_->scan_indices().size() ) {
        auto it = impl_->mzml_->scan_indices().begin() + rowid;
        return std::get< enum_scan_start_time >( it->first );
    }
    return -1;
}

int
DataReader::fcn( int64_t rowid ) const
{
    if ( 0 <= rowid && rowid < impl_->mzml_->scan_indices().size() ) {
        auto it = impl_->mzml_->scan_indices().begin() + rowid;
        return it->second->protocol_id();
    }
    return -1;
}

boost::any
DataReader::getData( int64_t rowid ) const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    return {};
}

std::shared_ptr< adcontrols::MassSpectrum >
DataReader::getSpectrum( int64_t rowid ) const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    return nullptr;
}

std::shared_ptr< adcontrols::MassSpectrum >
DataReader::readSpectrum( const const_iterator& it ) const
{
    if ( it->rowid() < impl_->mzml_->scan_indices().size() ) {
        const auto& datum = impl_->mzml_->scan_indices()[ it->rowid() ];
        return datum.second->toMassSpectrum( *datum.second );
    }
    return {};
}

std::shared_ptr< adcontrols::Chromatogram >
DataReader::getChromatogram( int fcn, double time, double width ) const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    return nullptr;
}

std::shared_ptr< adcontrols::MassSpectrum >
DataReader::coaddSpectrum( const_iterator&& first, const_iterator&& last ) const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ================== "
              << std::make_pair( first->time_since_inject(), last->time_since_inject() );

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

#if 0
    const auto& data = impl_->mzml_->scan_indices();
    size_t fst = first->rowid();
    size_t lst = last != end() ? last->rowid() : transformed.size();

    std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds > tp;
    std::chrono::nanoseconds elapsed_time( int64_t( std::get< scan_acquisition_time >( data.at( fst ) ) * 1e9 ) );
    if ( auto value = impl_->mzml_->find_global_attribute( "/experiment_date_time_stamp" ) ) {
        tp = time_stamp_parser{}( *value, true ) + elapsed_time; // ignore timezone, Shimadzu set TZ=0 (UTC), but time indicates local time
    }
    if ( auto value = impl_->mzml_->find_global_attribute( "/test_ionization_polarity" ) ) {
        if ( *value == "Positive Polarity" )
            ms->setPolarity( adcontrols::PolarityPositive );
        if ( *value == "Negative Polarity" )
            ms->setPolarity( adcontrols::PolarityNegative );
    }

    ms->resize( transformed.size() );
    ms->setAcquisitionMassRange( std::get< mass_range_min >(data.at( fst )), std::get< mass_range_max >(data.at( fst )) );
    auto& prop = ms->getMSProperty();
    prop.setTimeSinceInjection( std::get< scan_acquisition_time >( data.at( fst ) ) );
    prop.setTrigNumber( std::get< actual_scan_number >( data.at( fst ) ) );
    prop.setInstMassRange( { std::get< mass_range_min >(data.at( fst )), std::get< mass_range_max >(data.at( fst )) } );
    prop.setTimePoint( tp );

    for ( const auto& map: transformed ) {
        const auto& [ch,values] = map;
        ms->setMass( ch, values.first );
        ms->setIntensity( ch, std::accumulate( values.second.begin() + fst, values.second.begin() + lst, 0.0 ) );
        // ADDEBUG() << "\t" << std::make_tuple( ch, values.first, ms->intensity( ch ) );
    }
#endif
    return {};
}

std::shared_ptr< adcontrols::MassSpectrometer >
DataReader::massSpectrometer() const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    return {}; // spectrometer_;
}

adcontrols::DataInterpreter *
DataReader::dataInterpreter() const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    return {}; // interpreter_.get();
}

void
DataReader::handleCalibrateResultAltered() const
{
}
