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
#include "andims.hpp"
#include "timestamp.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adportable/debug.hpp>
#include <algorithm>
#include <chrono>
#include <numeric>

namespace adnetcdf {

    class DataReader::impl {
    public:
        impl( std::shared_ptr< const AndiMS > cdf ) : cdf_( cdf ) {
        }
        std::shared_ptr< const AndiMS > cdf_;

        constexpr const static boost::uuids::uuid uuid_ = { 0x07,0x5E,0x16,0x20
                                                            ,0x3D,0x70,0x42,0x75
                                                            ,0x93,0xDE,0x8D,0xC2
                                                            ,0x00,0x4A,0xDF,0x57 };
        static const std::string objtext_;
        static const std::string display_name_;
    };

    const std::string DataReader::impl::objtext_ = "1.adnetcdf.ms-cheminfo.com";
    const std::string DataReader::impl::display_name_ = "AndiMS";
}

using namespace adnetcdf;


DataReader::~DataReader()
{
}

DataReader::DataReader( const char * traceid
                        , std::shared_ptr< const AndiMS > cdf ) : adcontrols::DataReader( traceid )
                                                                , impl_( std::make_unique< impl >( cdf ) )
{
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
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    return 1;
}

size_t
DataReader::size( int fcn ) const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    return impl_->cdf_->data().size();
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
    auto& data = impl_->cdf_->data();
    auto it = std::lower_bound( data.begin(), data.end(), seconds
                                , [](const auto& a, const auto& b){ return std::get< scan_acquisition_time >( a ) < b; } );
    if ( it != data.end() ) {
        size_t rowid = std::distance( data.begin(), it );
        if ( closest && (it+1) != data.end() ) {
            if ( std::abs( std::get< scan_acquisition_time >(*it) - seconds ) > std::abs( std::get< scan_acquisition_time >(*(it+1)) - seconds ) )
                ++rowid;
        }
        ADDEBUG() << "-- DataReader " << __FUNCTION__ << " ================ pos: " << rowid;
        return adcontrols::DataReader_iterator( this, rowid, fcn );
    }
    return end();
}

double
DataReader::findTime( int64_t pos, IndexSpec ispec, bool exactMatch ) const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";

    assert( ispec == TriggerNumber );
    auto& data = impl_->cdf_->data();
    if ( 0 <= pos && pos <= data.size() )
        return std::get< scan_acquisition_time >( data[ pos ] );
    return -1.0;
}

std::shared_ptr< const adcontrols::Chromatogram >
DataReader::TIC( int fcn ) const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    return nullptr;
}

int64_t
DataReader::next( int64_t rowid ) const
{
    return next( rowid, 0 );
}

int64_t
DataReader::next( int64_t rowid, int fcn ) const
{
    if ( ( rowid + 1 ) < impl_->cdf_->data().size() )
        return ++rowid;
    return (-1);
}

int64_t
DataReader::prev( int64_t rowid ) const
{
    return prev( rowid, 0 );
}

int64_t
DataReader::prev( int64_t rowid, int fcn ) const
{
    if ( rowid >= 1 )
        return --rowid;
    return impl_->cdf_->data().size() - 1;
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
    if ( 0 <= rowid && rowid < impl_->cdf_->data().size() ) {
        const auto& datum = impl_->cdf_->data()[ rowid ];
        return std::get< scan_acquisition_time >( datum );
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
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ================== rowid = " << rowid;
    return -1;
}

int
DataReader::fcn( int64_t rowid ) const
{
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
    size_t idx = it->rowid();

    const auto& data = impl_->cdf_->data();
    const auto& transformed = impl_->cdf_->transformed();

    auto ms = std::make_shared< adcontrols::MassSpectrum >();
    ms->resize( transformed.size() );

    if ( auto value = impl_->cdf_->find_global_attribute( "/experiment_type" ) ) {
        if ( *value == "Centroided Mass Spectrum" ) // I'm not sure this is typo by Shimadzu or wrong specification by ASTM
            ms->setCentroid( adcontrols::CentroidNative );
    }
    std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds > tp;
    std::chrono::nanoseconds elapsed_time( int64_t( std::get< scan_acquisition_time >( data.at( idx ) ) * 1e9 ) );
    if ( auto value = impl_->cdf_->find_global_attribute( "/experiment_date_time_stamp" ) ) {
        tp = time_stamp_parser{}( *value, true ) + elapsed_time; // ignore timezone, Shimadzu set TZ=0 (UTC), but time indicates local time
    }
    if ( auto value = impl_->cdf_->find_global_attribute( "/test_ionization_polarity" ) ) {
        if ( *value == "Positive Polarity" )
            ms->setPolarity( adcontrols::PolarityPositive );
        if ( *value == "Negative Polarity" )
            ms->setPolarity( adcontrols::PolarityNegative );
    }
    ms->setAcquisitionMassRange( std::get< mass_range_min >(data.at( idx )), std::get< mass_range_max >(data.at( idx )) );
    auto& prop = ms->getMSProperty();
    prop.setTimeSinceInjection( std::get< scan_acquisition_time >( data.at( idx ) ) );
    prop.setTrigNumber( std::get< actual_scan_number >( data.at( idx ) ) );
    prop.setInstMassRange( { std::get< mass_range_min >(data.at( idx )), std::get< mass_range_max >(data.at( idx )) } );

    prop.setTimePoint( tp );

    for ( const auto& map: transformed ) {
        const auto& [ch,values] = map;
        ms->setMass( ch, values.first );
        ms->setIntensity( ch, *(values.second.begin() + idx) );
    }

    return ms;
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
    auto ms = std::make_shared< adcontrols::MassSpectrum >();
    const auto& transformed = impl_->cdf_->transformed();

    if ( auto value = impl_->cdf_->find_global_attribute( "/experiment_type" ) ) {
        if ( *value == "Centroided Mass Spectrum" ) { // I'm not sure this is typo by Shimadzu or wrong specification by ASTM
            ms->setCentroid( adcontrols::CentroidNative );
        }
    }
    const auto& data = impl_->cdf_->data();
    size_t fst = first->rowid();
    size_t lst = last != end() ? last->rowid() : transformed.size();

    std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds > tp;
    std::chrono::nanoseconds elapsed_time( int64_t( std::get< scan_acquisition_time >( data.at( fst ) ) * 1e9 ) );
    if ( auto value = impl_->cdf_->find_global_attribute( "/experiment_date_time_stamp" ) ) {
        tp = time_stamp_parser{}( *value, true ) + elapsed_time; // ignore timezone, Shimadzu set TZ=0 (UTC), but time indicates local time
    }
    if ( auto value = impl_->cdf_->find_global_attribute( "/test_ionization_polarity" ) ) {
        if ( *value == "Positive Polarity" )
            ms->setPolarity( adcontrols::PolarityPositive );
        if ( *value == "Negative Polarity" )
            ms->setPolarity( adcontrols::PolarityNegative );
    }

    // ADDEBUG() << __FUNCTION__ << "######### fst/lst=" << std::make_pair(fst, lst);

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

    return ms;
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
