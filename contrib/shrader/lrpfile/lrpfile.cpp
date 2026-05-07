/**************************************************************************
** Copyright (C) 2010-2026 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2026 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "lrpfile.hpp"
#include "instsetup.hpp"
#include "lrpcalib.hpp"
#include "lrphead2.hpp"
#include "lrphead3.hpp"
#include "lrpheader.hpp"
#include "lrptic.hpp"
#include "msdata.hpp"
#include "simions.hpp"
#include <adportable/debug.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <istream>
#include <sstream>

namespace shrader {

    class lrpfile::impl {
    public:
        impl() : loaded_( false )
               , header_( std::make_unique< lrpheader >() )
               , header2_( std::make_unique< lrphead2 >() )
               , header3_( std::make_unique< lrphead3 >() )
               , instsetup_( std::make_unique< shrader::instsetup >() )
               , lrpcalib_( std::make_unique< shrader::lrpcalib >() )
               , simions_( std::make_unique< shrader::simions >() )
               , lrptic_( std::make_unique< shrader::lrptic >() )
            {}
        bool loaded_;
        std::unique_ptr< shrader::lrpheader > header_;
        std::unique_ptr< shrader::lrphead2 > header2_;
        std::unique_ptr< shrader::lrphead3 > header3_;
        std::unique_ptr< shrader::instsetup > instsetup_;
        std::unique_ptr< shrader::lrpcalib > lrpcalib_;
        std::unique_ptr< shrader::simions > simions_;
        std::unique_ptr< shrader::lrptic > lrptic_;
        std::vector< std::shared_ptr< shrader::msdata > > msdata_;
    };
}

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };

using namespace shrader;

lrpfile::~lrpfile()
{
}

lrpfile::lrpfile() : impl_( std::make_unique< impl >() )
{
}


bool
lrpfile::load( std::istream& in, size_t fsize )
{
    if ( impl_->header_->load( in, fsize ) &&
         impl_->header2_->load( in, fsize ) &&
         impl_->header3_->load( in, fsize ) ) {

        if ( not impl_->instsetup_->load( in, fsize ) )
            ADDEBUG() << "## instsetup load failed ##";
        if ( not impl_->lrpcalib_->load( in, fsize ) )
            ADDEBUG() << "## lrpcalib load failed ##";
        if ( not impl_->simions_->load( in, fsize ) )
            ADDEBUG() << "## simions load failed ##";
        if ( not impl_->lrptic_->load( in, fsize ) )
            ADDEBUG() << "## lrptic load failed ##";

        ADDEBUG() << "## lrpcalib load: " << boost::json::value_from( *impl_->lrpcalib_ );

        if ( impl_->lrptic_ && *impl_->lrptic_ ) {
            for ( auto& tic: impl_->lrptic_->tic() ) {
                if ( auto data = std::make_shared< shrader::msdata >() ) {
                    if ( data->load( in, fsize, tic.ptr ) ) {
                        impl_->msdata_.emplace_back( std::move( data ) );
                    }
                }
            }
        }
        // dump( std::cerr, 2 );
        return true;
    }
    return false;
}

bool
lrpfile::xload( value_type t, const std::string& data )
{
    std::istringstream in( data );
    std::visit(overloaded{
            [](auto arg) { std::cout << arg << ' '; }
                , [&](lrpheader) { impl_->header_->load( in, data.size() ); }
                , [&](lrphead2)  { impl_->header2_->load( in, data.size() ); }
                , [&](lrphead3)  { impl_->header3_->load( in, data.size() ); }
                , [&](class instsetup) { impl_->instsetup_->load( in, data.size() ); }
                , [&](class lrpcalib)  { impl_->lrpcalib_->load( in, data.size() ); }
                , [&](class simions)   { impl_->simions_->load( in, data.size() ); }
                }, t);
    return true;
}

void
lrpfile::append( std::string&& data, std::string&& meta )
{
    if ( auto p = std::make_shared< shrader::msdata >() ) {
        std::istringstream in( data );
        p->load( in, data.size(), 0 );
        impl_->msdata_.emplace_back( std::move( p ) );
    }
    impl_->lrptic_->append( std::move( meta ) );

}


lrpfile::operator bool() const
{
    return impl_->loaded_;
}

const shrader::msdata *
lrpfile::operator []( size_t idx ) const
{
    if ( impl_->msdata_.size() > idx )
        return impl_->msdata_[ idx ].get();
    return 0;
}

const shrader::lrpheader&
lrpfile::header() const
{
    return *impl_->header_;
}

const shrader::lrphead2&
lrpfile::header2() const
{
    return *impl_->header2_;
}

const shrader::lrphead3&
lrpfile::header3() const
{
    return *impl_->header3_;
}

const shrader::instsetup&
lrpfile::instsetup() const
{
    return *impl_->instsetup_;
}

const shrader::lrpcalib&
lrpfile::lrpcalib() const
{
    return *impl_->lrpcalib_;
}

const shrader::simions&
lrpfile::simions() const
{
    return *impl_->simions_;
}

const shrader::lrptic&
lrpfile::lrptic() const
{
    return *impl_->lrptic_;
}

const std::vector< std::shared_ptr< shrader::msdata > >&
lrpfile::msdata() const
{
    return impl_->msdata_;
}

size_t lrpfile::number_of_spectra() const
{
    return impl_->msdata_.size();
}

std::string
lrpfile::time_of_injection() const
{
    std::tm tm{};
    std::istringstream in( header().analdate() );
    in >> std::get_time( &tm, "%m/%d/%Y" );

    std::tm lt{0};
    time_t t = time(0);
    localtime_r(&t, &lt);

    // "analdate":"5/30/2025" "analtime":"17:17:24" --> 2025-05-30T17:17:24+0900 (in case at JST area)
    return std::format( "{:04d}-{:02d}-{:02d}T{}{:+03d}{:02d}"
                        , tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday
                        , header().analtime()
                        , lt.tm_gmtoff / 3600
                        , (lt.tm_gmtoff / 60) % 60 );
}

ticc_t
lrpfile::get_ticc() const
{
    if ( lrptic() && lrptic() ) {
        ticc_t ticc;
        for ( auto& tic : lrptic().tic() )
            ticc.emplace_back( double( tic.time ) / 1000.0, tic.intensity, tic.ptr, tic.overload );
        return ticc;
    }
    return {};
}

void
lrpfile::tic_set_loaded( bool f )
{
    impl_->lrptic_->set_loaded( f );
}

void
lrpfile::dump( std::ostream& of, size_t limit ) const
{
    if ( impl_->header_ && header() ) {
        of << "----------------------------------------------" << std::endl;
        of << boost::json::value_from( header() )
           << std::endl;
    }
    if ( impl_->header2_ && header2() ) {
        of << "----------------------------------------------" << std::endl;
        of << boost::json::value_from( header2() )
           << std::endl;
    }
    if ( impl_->header3_ && header3() ) {
        of << "----------------------------------------------" << std::endl;
        of << boost::json::value_from( header3() )
           << std::endl;
    }

    if ( impl_->instsetup_ && instsetup() ) {
        of << "----------------------------------------------" << std::endl;
        of << boost::json::value_from( instsetup() )
           << std::endl;
    }


    if ( impl_->lrpcalib_ && lrpcalib() ) {
        of << "---------------------- lrpcalib ------------------------" << std::endl;
        of << boost::json::value_from( lrpcalib() ) << std::endl;
    }
#if 0
    if ( impl_->lrptic_ && lrptic() ) {
        of << "---------------------- lrptic ------------------------" << std::endl;
        of << "----------- TIC -------------- " << impl_->lrptic_->tic().size() << std::endl;
        of << "nexttpr:\t" << impl_->lrptic_->nextptr() << std::endl;
        for ( auto& tic: impl_->lrptic_->tic() ) {
            of << boost::format( "\ttime: %.3fs\tintens: %d\toffs: 0x%x\toverload: %d\n" )
                % (double( tic.time ) / 1000.0) % tic.intensity % tic.ptr % tic.overload;
        }
        of << "----------- END of TIC --------------" << std::endl;
    }
    of << std::endl;
#endif
    of << "---------------------- msdata ------------------------" << std::endl;
    of << "total: " << impl_->msdata_.size() << " spectra." << std::endl;
    size_t scan = 0;
    for ( auto& msdata : impl_->msdata_ ) {
        if ( scan++ >= limit )
            break;
        of << "===================== scan = " << scan << " =========================" << std::endl;

        if ( msdata ) {
            for ( const auto& block: msdata->blocks() )
                of << boost::json::value_from( block ) << std::endl;
#if 0
            size_t nblocks = msdata->size();
            of << "scan: " << ++scan;
            if ( msdata->is_profile( msdata->flags( 0 ) ) ) {
                of << " profile ";
            }
            else {
                of << " centroid (or SIM, SRM)";
            }
            of << std::endl;

            for ( int blk = 0; blk < nblocks; ++blk ) {
                of << boost::format( "blk# %2d\tscan# %3d\tnions: %2d" ) % blk % msdata->scan( blk ) % msdata->nions( blk );
                auto xrange = std::make_pair( msdata->xlow( blk ), msdata->xhigh( blk ) );
                auto flags = msdata->flags( blk );
                auto nions = msdata->nions( blk );

                of << boost::format( "\tx-range: (%5d, %5d) d = %2d" )
                    % xrange.first % xrange.second % (xrange.second - xrange.first);

                if ( msdata->is_mass_array( flags ) ) {
                    of << " := m/z(" << double( xrange.first ) / 65536 << ", " << double( xrange.second ) / 65536 << ")";
                } else {
                    auto range = std::make_pair( double( xrange.first ) / 16, double( xrange.second ) / 16 );
                    of << boost::format( "\t:= time( %.3f, %.3f ) dt: %.7e" )
                        % range.first % range.second % ( ( range.second - range.first ) / ( nions - 1 ) );
                }
                of << std::endl;
            }
#endif
        }
    }
}
