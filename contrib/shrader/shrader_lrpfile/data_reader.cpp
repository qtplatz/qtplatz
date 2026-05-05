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

#include <adutils/datafile_signature.hpp>
#include "data_reader.hpp"
#include "msdata.hpp"
#include "helper.hpp"
#include <chrono>
#include "chromatogram.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/bzip2.hpp>
#include <adportable/debug.hpp>
#include <adportable/debug.hpp>
#include <adportable/iso8601.hpp>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <format>
#include <instsetup.hpp>
#include <lrpcalib.hpp>
#include <lrpfile.hpp>
#include <lrphead2.hpp>
#include <lrphead3.hpp>
#include <lrpheader.hpp>
#include <lrptic.hpp>
#include <simions.hpp>
#include <numeric>
#include <sstream>
#include <stdexcept>

namespace shrader {
    namespace lrp_ex {

        class data_reader::impl {
        public:
            impl() : lrpfile_( std::make_shared< lrpfile >() ) {
            }
            std::weak_ptr< adfs::sqlite > db_;
            int fcn_;

            std::shared_ptr< lrpfile > lrpfile_;
            std::string traceid_;

            constexpr const static boost::uuids::uuid uuid_ = {
                0xd5, 0x0f, 0x25, 0xd9, 0x0b, 0xf4, 0x4f, 0xc1
                , 0x88, 0xfa, 0xe2, 0xe3, 0xc2, 0x9d, 0xfb, 0xc0
            };
            const static std::string objtext__;
            std::string display_name_;
        };

        std::string const data_reader::impl::objtext__ = "1.adfs.lrp.ms-cheminfo.com";
    }
}

using namespace shrader::lrp_ex;

data_reader::~data_reader()
{
}

data_reader::data_reader( const char * traceid )
    : adcontrols::DataReader( traceid )
    , impl_( std::make_unique< impl >() )
{
    impl_->traceid_ = traceid;
    ADDEBUG() << "#### << " << __FUNCTION__ << std::format( " data_reader({})", traceid );
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

    impl_->db_ = db;
    if ( auto db = impl_->db_.lock() ) {
        adfs::stmt sql( *db );

        using namespace adutils::data_signature;
        std::map< std::string, value_t > sigs;
        sql >> sigs;
        if ( sigs.find( "lrpheader" ) != sigs.end() ) {
            impl_->lrpfile_->xload( lrpheader{}, string_to_block( std::get< std::string >( sigs["lrpheader"] ) ) );
        }
        if ( sigs.find( "lrpheader2" ) != sigs.end() ) {
            impl_->lrpfile_->xload( lrphead2{}, string_to_block( std::get< std::string >( sigs["lrpheader2"] ) ) );
        }
        if ( sigs.find( "lrpheader3" ) != sigs.end() ) {
            impl_->lrpfile_->xload( lrphead3{}, string_to_block( std::get< std::string >( sigs["lrpheader3"] ) ) );
        }
        if ( sigs.find( "instsetup" ) != sigs.end() ) {
            impl_->lrpfile_->xload( instsetup{}, string_to_block( std::get< std::string >( sigs["instsetup"] ) ) );
        }
        if ( sigs.find( "calib" ) != sigs.end() ) {
            ADDEBUG() << "--------- calib found ---------";
            impl_->lrpfile_->xload( lrpcalib{}, string_to_block( std::get< std::string >( sigs["calib"] ) ) );
        }
        ADDEBUG() << boost::json::value_from( impl_->lrpfile_->header() );
        ADDEBUG() << boost::json::value_from( impl_->lrpfile_->header2() );
        ADDEBUG() << boost::json::value_from( impl_->lrpfile_->header3() );
        ADDEBUG() << boost::json::value_from( impl_->lrpfile_->instsetup() );
        ADDEBUG() << boost::json::value_from( impl_->lrpfile_->lrpcalib() );

        sql.prepare( "SELECT rowid,elapsed_time,npos,fcn,events,data,meta FROM AcquiredData WHERE objuuid=?" );
        sql.bind(1) = impl::uuid_;
        while ( sql.step() == adfs::sqlite_row ) {
            try {
                auto rowid = sql.get_column_value< int64_t >( 0 );
                auto elapsed_time = sql.get_column_value< int64_t >( 1 ); // ns
                auto pos = sql.get_column_value< int64_t >( 2 );
                auto fcn = sql.get_column_value< int64_t >( 3 );
                auto events = sql.get_column_value< int64_t >( 4 );

                auto data = bzip2_decompress( sql.get_column_value< adfs::blob >( 5 ) );
                std::istringstream is( data );

                auto meta = bzip2_decompress( sql.get_column_value< adfs::blob >( 6 ) );

                // ADDEBUG() << std::make_tuple( rowid, elapsed_time, pos, fcn, events, data.size(), meta.size() );

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
    ADDEBUG() << "## DataReader " << __FUNCTION__ << " ==================";
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
    return impl_->objtext__;
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
    auto get_blocks = [&]( int64_t rowid ){
        return impl_->lrpfile_->msdata().at( rowid )->blocks();
    };

    auto header_mass_range = [&]()->std::pair<double,double>{
        return {double(impl_->lrpfile_->instsetup().lmasslim())/65536.0
                , double( impl_->lrpfile_->instsetup().umasslim()) /65536.0 };
    };

    if ( rowid > impl_->lrpfile_->msdata().size() )
        return {};
#if 0
    if ( impl_->lrpfile_->lrpcalib() ) {
        for ( size_t i = 0; i < impl_->lrpfile_->lrpcalib().cal_size; ++i ) {
            auto cal = impl_->lrpfile_->lrpcalib().cal_data( i );
            if ( std::get<0>(cal) ) {
                ADDEBUG() << std::format( "Calibration: mass = {}, intens = {}, coeff a = {}, b = {}\n"
                                          , double(std::get< cal_mass >( cal ))/65535.0
                                          , std::get< cal_intens > ( cal )
                                          , std::get< cal_coeff_a > ( cal )
                                          , std::get< cal_coeff_b > ( cal ) )
                          << ADDEBUG() << boost::json::value_from( get_blocks( rowid ).at( 0 ) );
            }
        }
    }
#endif
    auto mass_range = header_mass_range();
    // ADDEBUG() << "mass_range: " << mass_range;

    const auto& blocks = get_blocks( rowid );
    auto numSamples = std::accumulate( blocks.begin(), blocks.end()
                                       , size_t(0), [](const auto& a, const auto& b){
                                           return a + b.nions; });
    // ADDEBUG() << "nSamples: " << numSamples << ", " << get_blocks( rowid ).size();

    auto cal = impl_->lrpfile_->lrpcalib().cal_data( 0 );

    auto mass_at = [&]( size_t idx )->double{
        // assume time squared scan law (TOF Eq.)
        const double f = double( idx ) / double( numSamples - 1 );
        const double s0 = std::sqrt( mass_range.first );
        const double s1 = std::sqrt( mass_range.second );
        const double s = s0 + f * ( s1 - s0 );
        return s * s;
    };

    auto local_mass_at = []( size_t idx, const detail::block& blk ){
        auto range = std::make_pair( double(blk.xlow) / 16.0, double(blk.xhigh) / 16.0 );
        return range.first + idx * (range.second - range.first) / (blk.nions - 1);
    };

    std::vector< std::pair< double, double > > vec;
    size_t idx{0};
    for ( const auto& block: get_blocks( rowid ) ) {
        if ( block.xlow == 0 && block.xhigh == 0 ) {
            for ( size_t i = 0; i < block.nions; ++i ) {
                vec.emplace_back( mass_at( idx++ ), block.u.profile[ i ] );
            }
        } else { // legacy code
            for ( size_t i = 0; i < block.nions; ++i )
                vec.emplace_back( local_mass_at( i, block ), block.u.profile[ i ] );
        }
    }
    if ( auto ms = std::make_shared< adcontrols::MassSpectrum >() ) {
        for ( const auto& datum: vec )
            *ms << datum;
        ms->setAcquisitionMassRange( mass_range.first, mass_range.second );
        ms->getMSProperty().setTrigNumber( blocks.at(0).scan );
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
