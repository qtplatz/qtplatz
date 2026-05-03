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

#include "data_reader.hpp"
#include "msdata.hpp"
#include <lrpfile.hpp>
#include <lrptic.hpp>
#include "chromatogram.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adportable/debug.hpp>
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
                , fcn_( fcn ) {
                objtext_ = std::format("{}.lrpfile.ms-cheminfo.com",  fcn_ );
            }
            std::shared_ptr< const lrpfile > lrpfile_;
            int fcn_;
            std::string objtext_;
            std::string traceid_;

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
    // lrpfile->dump( std::cerr, 1 );
    // auto jv = boost::json::parse( traceid );
    // impl_->traceid_ = boost::json::serialize( jv );
    // impl_->scan_protocol_ = boost::json::value_to< scan_protocol >( jv );
    // impl_->protocol_key_ = impl_->scan_protocol_.protocol_key();

    // std::ostringstream o;
    // o << boost::format("MS%d") % impl_->scan_protocol_.ms_level();
    // if ( impl_->scan_protocol_.ms_level() == 2 ) { // pscan
    //     o << boost::format( " %.1f[%.1fV] " )
    //         % impl_->scan_protocol_.precursor_mz()
    //         % impl_->scan_protocol_.collision_energy();
    // }
    // o << (impl_->scan_protocol_.polarity() == polarity_negative ? "(−)"
    //       : impl_->scan_protocol_.polarity() == polarity_positive ? "(+)"
    //       : "(?)");
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
    ADDEBUG() << "############# " << __FUNCTION__ << " ##############";
    return impl_->traceid_;
}

std::string
data_reader::abbreviated_display_name() const
{
    ADDEBUG() << "############# " << __FUNCTION__ << " ##############";
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
    ADDEBUG() << "############# " << __FUNCTION__ << " ##############";
    return 1; //impl_->mzml_->getFunctionCount();
}

size_t
data_reader::size( int fcn /* segmented fcn */ ) const
{
    ADDEBUG() << "############# " << __FUNCTION__ << " ############## " << impl_->lrpfile_->number_of_spectra();;
    // return std::accumulate( impl_->mzml_->scan_indices().begin()
    //                         , impl_->mzml_->scan_indices().end()
    //                         , 0
    //                         , [&]( const auto& a, const auto& b ){
    //                             return (b.second->protocol_id() == impl_->fcn_ ? 1 : 0) + a;
    //                         });
    // return impl_->mzml_->getSpectrumCount( fcn );
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
        ADDEBUG() << "############# " << __FUNCTION__ << " ##############"
                  << std::format( "seconds={}, fcn={}", seconds, fcn);
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
    ADDEBUG() << "############# " << __FUNCTION__ << " ##############";
    return next( rowid, impl_->fcn_ );
}

int64_t
data_reader::next( int64_t rowid, int fcn ) const
{
    ADDEBUG() << "############# " << __FUNCTION__ << " ##############";
    if ( (rowid + 1) < impl_->lrpfile_->number_of_spectra() ) {
        return ++rowid;
    }
    // if ( (rowid + 1) < impl_->mzml_->scan_indices().size() ) {
    //     auto it = impl_->mzml_->scan_indices().begin() + rowid + 1;
    //     while ( it != impl_->mzml_->scan_indices().end() ) {
    //         if ( it->second->protocol_id() == fcn )
    //             return std::distance( impl_->mzml_->scan_indices().begin(), it );
    //         ++it;
    //     }
    // }
    return (-1);
}

int64_t
data_reader::prev( int64_t rowid ) const
{
    ADDEBUG() << "############# " << __FUNCTION__ << " ##############";
    return prev( rowid, impl_->fcn_ );
}

int64_t
data_reader::prev( int64_t rowid, int fcn ) const
{
    ADDEBUG() << "############# " << __FUNCTION__ << " ##############";
    if ( rowid )
        return --rowid;
    return 0;
}

int64_t
data_reader::pos( int64_t rowid ) const
{
    ADDEBUG() << "############# " << __FUNCTION__ << " ##############";
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
    ADDEBUG() << "############# " << __FUNCTION__ << " ##############";
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
    auto get_blocks = [&]( int64_t rowid ){
        return impl_->lrpfile_->msdata().at( rowid )->blocks();
    };
    auto header_mass_range = [&]()->std::pair<double,double>{
        return {double(impl_->lrpfile_->instsetup().lmasslim())/65536.0
                , double( impl_->lrpfile_->instsetup().umasslim()) /65536.0 };
    };

    if ( rowid > impl_->lrpfile_->msdata().size() )
        return {};

    auto mass_range = header_mass_range();
    ADDEBUG() << "mass_range: " << mass_range;

    for ( const auto& block: get_blocks( rowid ) ) {
        ADDEBUG() << boost::json::value_from( block );
    }

#if 0
    if ( auto msdata = (*impl_->lrpfile_)[ rowid ] ) {
        std::vector< double > time, intens;
        if ( impl_->lrpfile_->getMS( *msdata, time, intens ) ) {
            if ( auto ms = std::make_shared< adcontrols::MassSpectrum >() ) {
                ms->resize( time.size() );
                ms->setMassArray( time.data() );
                ms->setIntensityArray( intens.data() );
                // for ( size_t i = 0; i < ms->size(); ++i ) {
                //     ADDEBUG() << std::make_tuple( ms->mass( i ), ms->intensity( i ) );
                // }
                ms->setAcquisitionMassRange( time.front(), time.back() );
                return ms;
            }
        }
    }
#endif
    return nullptr;
}

std::shared_ptr< adcontrols::MassSpectrum >
data_reader::readSpectrum( const const_iterator& it ) const
{
    if ( it->rowid() < impl_->lrpfile_->number_of_spectra() ) {
        if ( auto reader = it.dataReader() ) {
            ADDEBUG() << "## DataReader " << __FUNCTION__ << " =============== found reader: " << reader->display_name();
            return reader->getSpectrum( it->rowid() );
        }
    }
    // if ( it->rowid() < impl_->mzml_->scan_indices().size() ) {
    //     const auto& datum = impl_->mzml_->scan_indices()[ it->rowid() ];
    //     return datum.second->toMassSpectrum( *datum.second );
    // }
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
    ADDEBUG() << "############# " << __FUNCTION__ << " ##############";
    auto ms = std::make_shared< adcontrols::MassSpectrum >();
    for ( auto it = first; it != last; it++ ) {
        if ( auto msdata = (*impl_->lrpfile_)[it->rowid()] ) {
            std::vector< double > time, intens;
            if ( impl_->lrpfile_->getMS( *msdata, time, intens ) ) {
                if ( ms->size() == 0 ) {
                    ms->resize( time.size() );
                    ms->setMassArray( time.data() );
                    ms->setIntensityArray( intens.data() );
                    ms->setAcquisitionMassRange( time.front(), time.back() );
                } else {
                    for ( size_t i = 0; i < ms->size(); ++i ) {
                        ms->setIntensity( i, ms->intensity(i) + intens[i] );
                    }
                }
            }
        }
    }
    return ms;

    // auto it = impl_->mzml_->scan_indices().begin() + first.rowid();
    // if ( it->second->protocol_id() != impl_->fcn_ )
    //     throw std::invalid_argument("protocol id missmatch");

    // auto ms = it->second->toMassSpectrum( *it->second );
    // std::vector< double > intensities ( ms->size(), 0 );
    // last++; // advance for pointing end

    // for ( auto it = first; it != last; it++ ) {
    //     const auto sp = impl_->mzml_->scan_indices()[ it.rowid() ].second;
    //     if ( sp->protocol_id() != impl_->fcn_ ) {
    //         ADDEBUG() << "Internal error -- protocol id missmatch " << std::format("{} != {}", sp->protocol_id(), impl_->fcn_);
    //         continue;
    //     }
    //     const auto& secondi = sp->dataArrays().second;
    //     std::visit([&](auto arg){
    //         for ( size_t i = 0; i < secondi.length(); ++i )
    //             intensities[i] += *arg++;
    //     }, secondi.data() );
    // }
    // ms->setIntensityArray( std::move( intensities ) );
    // return ms;
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
