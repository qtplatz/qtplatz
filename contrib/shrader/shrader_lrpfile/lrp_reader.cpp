/**************************************************************************
 ** Copyright (C) 2010-2026 Toshinobu Hondo, Ph.D.
 ** Copyright (C) 2013-2026 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "lrp_reader.hpp"
#include "msdata.hpp"
#include <chrono>
#include <lrpcalib.hpp>
#include <lrpfile.hpp>
#include <lrptic.hpp>
#include <mass_assign.hpp>
#include "chromatogram.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adportable/debug.hpp>
#include <adportable/iso8601.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>
#include <boost/format.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <format>
#include <iostream>
#include <lrpheader.hpp>
#include <lrphead2.hpp>
#include <lrphead3.hpp>
#include <instsetup.hpp>
#include <lrpcalib.hpp>
#include <simions.hpp>
#include <lrptic.hpp>

namespace shrader {
    namespace local {

        class data_reader::impl {
        public:
            impl( std::shared_ptr< const lrpfile > file, int fcn )
                : lrpfile_( file )
                , fcn_( fcn )
                , objtext_( std::format("{}.lrp.ms-cheminfo.com",  fcn_ ) ) {
            }
            std::shared_ptr< const lrpfile > lrpfile_;
            int fcn_;
            std::string traceid_;
            const std::string objtext_;

            constexpr const static boost::uuids::uuid uuid_ = {
                0xA8, 0xD2, 0xA0, 0xCA, 0xEC, 0x3E, 0x4D, 0xE0,
                0xAE, 0x9E, 0xFA, 0x53, 0x82, 0x60, 0xAC, 0xAD
            };
            std::string display_name_;
        };
    }
}

using namespace shrader::local;

data_reader::~data_reader()
{
}

data_reader::data_reader( const char * traceid
                          , int fcn
                          , std::shared_ptr< const shrader::lrpfile > lrpfile )
    : adcontrols::DataReader( traceid )
    , impl_( std::make_unique< impl >( lrpfile, fcn ) )
{
    impl_->traceid_ = traceid;

    ADDEBUG() << "##########################################";
    ADDEBUG() << boost::json::value_from( lrpfile->header() );
    ADDEBUG() << boost::json::value_from( lrpfile->header2() );
    ADDEBUG() << boost::json::value_from( lrpfile->header3() );
    impl_->display_name_ = std::format( "lrp.{}", boost::trim_copy( lrpfile->header().instrument() ) );
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
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
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
    return impl_->lrpfile_->number_of_spectra();
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
    const auto& tic = impl_->lrpfile_->lrptic().tic();;
    auto it = std::lower_bound( tic.begin(), tic.end(), int(seconds * 1000), []( const auto& a, int value ){
        return a.time < value;
    });

    if ( it != tic.end() ) {
        auto rowid = std::distance( tic.begin(), it );
        if ( closest && (it+1) != tic.end() ) {
            if ( std::abs( it->time - int(seconds*1000))
                 > std::abs( it->time - int(seconds*1000)) )
                ++rowid;
        }
        auto iter = adcontrols::DataReader_iterator( this, rowid, impl_->fcn_ );
        return iter;
    }
    ADDEBUG() << "\t## " << __FUNCTION__ << std::format( " -- {} s on fcn {} data not found", seconds, fcn );
    return end();
}

double
data_reader::findTime( int64_t pos, IndexSpec ispec, bool exactMatch ) const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
    assert( ispec == TriggerNumber );
    const auto& tic = impl_->lrpfile_->lrptic().tic();
    if ( 0 <= pos && pos <= tic.size() )
        return tic[ pos ].time / 1000.0;
    return -1.0;
}

std::shared_ptr< const adcontrols::Chromatogram >
data_reader::TIC( int fcn ) const
{
    ADDEBUG() << "############# " << __FUNCTION__ << " ##############";
    // if ( auto chro = std::shared_ptr< adcontrols::Chromatogram >() ) {
    //     impl_->mzml_->getTIC( fcn, *chro );
    //     return chro;
    // }
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
    if ( (rowid + 1) < impl_->lrpfile_->number_of_spectra() ) {
        return ++rowid;
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
    if ( rowid )
        return --rowid;
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
    ADDEBUG() << "############# " << __FUNCTION__ << " ##############";
    if ( 0 <= rowid && rowid < impl_->lrpfile_->number_of_spectra() ) {
        return double( impl_->lrpfile_->lrptic().tic()[ rowid ].time ) / 1000.0;
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
    const auto& tic = impl_->lrpfile_->lrptic().tic();;
    if ( rowid < tic.size() ) {
        return double( tic[rowid].time ) / 1000.0;
    }
    return -1;
}

int
data_reader::fcn( int64_t rowid ) const
{
    ADDEBUG() << "############# " << __FUNCTION__ << " ##############";
    if ( 0 <= rowid && rowid < impl_->lrpfile_->number_of_spectra() ) {
        return 1;
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
    auto header_mass_range = [&]()->std::pair<double,double>{
        return {double(impl_->lrpfile_->instsetup().lmasslim())/65536.0
                , double( impl_->lrpfile_->instsetup().umasslim()) /65536.0 };
    };
    const auto upperdrive = impl_->lrpfile_->instsetup().upperdrive();
    const auto stepsize  = impl_->lrpfile_->instsetup().stepsize();
    const auto clockbaud = impl_->lrpfile_->instsetup().clockbaud();
    const auto scanlaw = impl_->lrpfile_->instsetup().scanlaw();
    double dt = stepsize * clockbaud;
    ADDEBUG() << "############# spectral time step: " << std::format( "{}, stepsize={}, clockbaud={}", dt, stepsize, clockbaud );

    if ( rowid > impl_->lrpfile_->msdata().size() )
        return {};

    if ( rowid < 0 || rowid >= impl_->lrpfile_->msdata().size()) {
        return {};
    }
    auto y = impl_->lrpfile_->msdata().at( rowid )->intensities();
    auto numSamples = y.size();
    auto mass_range = header_mass_range();

    auto mass_at = [&]( size_t idx )->double{
        // assume time squared scan law (TOF Eq.)
        const double f = double( idx ) / double( numSamples - 1 );
        const double s0 = std::sqrt( mass_range.first );
        const double s1 = std::sqrt( mass_range.second );
        const double s = s0 + f * ( s1 - s0 );
        return s * s;
    };

    shrader::mass_assign mass_assigner( *impl_->lrpfile_ );

    std::vector< std::pair< double, double > > vec;
    for ( size_t i = 0; i < y.size(); ++i ) {
        if ( i < 10 )
            ADDEBUG() << i << "\t" << mass_assigner( i ) << ", " << mass_at(i);
        vec.emplace_back( mass_at( i ), y.at( i ) );
        // vec.emplace_back( mass_assigner( i ), y.at( i ) );
    }

    if ( auto ms = std::make_shared< adcontrols::MassSpectrum >() ) {
        for ( const auto& datum: vec )
            *ms << datum;
        ms->setAcquisitionMassRange( mass_range.first, mass_range.second );
        ms->getMSProperty().setTrigNumber( impl_->lrpfile_->msdata().at( rowid )->blocks().at(0).scan );
        ms->getMSProperty().setInstMassRange( header_mass_range() );
        if ( auto& lrptic = impl_->lrpfile_->lrptic() ) {
            const auto milliseconds = lrptic.tic().at( rowid ).time;
            ms->getMSProperty().setTimeSinceInjection( milliseconds, adcontrols::metric::milli );
            auto time_of_injection = impl_->lrpfile_->time_of_injection();
            if ( auto tp = adportable::iso8601::parse( time_of_injection.begin(), time_of_injection.end() ) ) {
                ms->getMSProperty().setTimePoint( *tp + std::chrono::milliseconds( milliseconds ) );
            }
        }
        return ms;
    }
    return {};
}

std::shared_ptr< adcontrols::MassSpectrum >
data_reader::readSpectrum( const const_iterator& it ) const
{
    if ( it->rowid() < impl_->lrpfile_->number_of_spectra() ) {
        if ( auto reader = it.dataReader() ) {
            return reader->getSpectrum( it->rowid() );
        }
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
    if ( auto ms = getSpectrum( first->rowid() ) ) {
        ++first;
        for ( auto it = first; it != last; ++it ) {
            auto rhs = getSpectrum( it->rowid() );
            *ms += *rhs;
        }
        return ms;
    }
    return {};
}

std::shared_ptr< adcontrols::MassSpectrometer >
data_reader::massSpectrometer() const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " NOT IMPL ==================";
    return {}; // spectrometer_;
}

adcontrols::DataInterpreter *
data_reader::dataInterpreter() const
{
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " NOT IMPL ==================";
    return {}; // interpreter_.get();
}

void
data_reader::handleCalibrateResultAltered() const
{
}
