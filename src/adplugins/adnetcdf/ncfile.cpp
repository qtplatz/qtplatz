// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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

#include "ncfile.hpp"
#include "dimension.hpp"
#include "variable.hpp"
#include "attribute.hpp"
#include <adportable/debug.hpp>
#include <netcdf.h>
#include <filesystem>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>

namespace adnetcdf {
    namespace netcdf {
        ncfile
        open( const std::filesystem::path& path, open_mode mode )
        {
            return ncfile{ path, mode };
        }

        ncfile
        open( const boost::filesystem::path& path, open_mode mode )
        {
            return ncfile{ std::filesystem::path( path.string() ), mode };
        }
        //////////////////////
    }
}

/////////////////////// ncfile /////////////////////
using namespace adnetcdf;
using namespace adnetcdf::netcdf;

ncfile::~ncfile()
{
    nc_close( ncid_ );
}

ncfile::ncfile() : rcode_(-1)
                 , ncid_(0)
                 , ndims_( 0 )
                 , nvars_( 0 )
                 , ngatts_( 0 )
                 , unlimdimid_( -1 )
{
}

ncfile::ncfile( const std::filesystem::path& path
                , open_mode mode ) : path_( path )
                                   , rcode_( -1 )
                                   , ncid_( 0 )
                                   , ndims_( 0 )
                                   , nvars_( 0 )
                                   , ngatts_( 0 )
                                   , unlimdimid_( -1 )
{
    rcode_ = nc_open( path.string().c_str(), int( mode), &ncid_ );
    rcode_ = nc_inq( ncid_, &ndims_, &nvars_, &ngatts_, &unlimdimid_ );
}

int32_t ncfile::rcode() const      { return rcode_; }
int32_t ncfile::ncid() const       { return ncid_; }
int32_t ncfile::ndims() const      { return ndims_; }
int32_t ncfile::nvars() const      { return nvars_; }
int32_t ncfile::ngatts() const     { return ngatts_; }
int32_t ncfile::unlimdimid() const { return unlimdimid_; }

const std::filesystem::path& ncfile::path() const { return path_; }

std::optional< int >
ncfile::inq_format() const
{
    int formatn;
    if ( nc_inq_format( ncid_, &formatn ) == NC_NOERR )
        return formatn;
    return {};
}

std::optional< std::pair<int, int> >
ncfile::inq_format_extended() const
{
    int nc_extended, nc_mode;
    if ( nc_inq_format_extended( ncid_, &nc_extended, &nc_mode ) == NC_NOERR )
        return {{ nc_extended, nc_mode }};
    return {};
}

std::pair< int, std::string >
ncfile::kind() const
{
    static const std::pair<int, std::string > list [] = {
        { 0,           "unrecognized" }  // 0
        , { /* 1 */ NC_FORMAT_CLASSIC,         "classic" }  // 1
        , { /* 2 */ NC_FORMAT_64BIT_OFFSET,    "64-bit offset" } // 2
        , { /* 3 */ NC_FORMAT_CDF5,            "cdf5" }  // 3
        , { /* 4 */ NC_FORMAT_NETCDF4,         "netCDF-4"}  // 4
        , { /* 5 */ NC_FORMAT_NETCDF4_CLASSIC, "netCDF-4 classic model" } }; // 5

    if ( auto nc_kind = inq_format() ) {
        auto it = std::find_if( list, list + sizeof(list)/sizeof(list[0]), [&](const auto& a){ return std::get<0>(a) == *nc_kind; } );
        if ( it != list + sizeof(list)/sizeof(list[0]) )
            return *it;
        return { *nc_kind, "unrecognized file format" };
    }
    return {};
}

std::tuple< int, uint16_t, std::string >
ncfile::kind_extended() const
{
    static const std::pair<int, std::string > list [] = {
        { NC_FORMATX_UNDEFINED,   "unknown" } // (0)
        , { NC_FORMATX_NC3,       "classic" } // (1)
        , { NC_FORMATX_NC_HDF5,   "HDF5" }    // (2)
        , { NC_FORMATX_NC_HDF4,   "HDF4" }    // (3)
        , { NC_FORMATX_PNETCDF,   "PNETCDF" } // (4)
        , { NC_FORMATX_DAP2,      "DAP2" }    // (5)
        , { NC_FORMATX_DAP4,      "DAP4" }    // (6)
        , { NC_FORMATX_UDF0,      "UDF0" }    // (8)
        , { NC_FORMATX_UDF1,      "UDF1" }    // (9)
        , { NC_FORMATX_NCZARR,    "NCZRR" }   // (10)
    };
    if ( auto nc_kind = inq_format() ) {
        if ( auto pair = inq_format_extended() ) {
            auto [nc_extended, nc_mode] = *pair;
            if ( *nc_kind == NC_FORMAT_NC3 ) {
                if ( nc_mode & NC_CDF5 )
                    return {*nc_kind, nc_mode, "64-bit data"};
                else if ( nc_mode & NC_64BIT_OFFSET )
                    return {*nc_kind, nc_mode, "64-bit offset"};
            }
            auto it = std::find_if( list, list + sizeof(list)/sizeof(list[0])
                                    , [&](const auto& a){ return std::get<0>(a) == *nc_kind; } );

            if ( it != list + sizeof(list)/sizeof(list[0]) )
                return { *nc_kind, nc_mode, std::get<1>(*it) };
            return { *nc_kind, nc_mode, "unrecognized" };
        }
    }
    return {};
}

const std::vector< dimension >&
ncfile::dims() const
{
    if ( dims_.empty() ) {
        for ( int dimid = 0; dimid < ndims_; ++dimid ) {
            if ( auto dim = inq_dim( dimid ) )
                dims_.emplace_back( *dim );
        }
    }
    return dims_;
}

const std::vector< variable >&
ncfile::vars() const
{
    if ( vars_.empty() ) {
        for ( int varid = 0; varid < nvars_; ++varid ) {
            if ( auto var = inq_var( varid ) ) {
                vars_.emplace_back( *var );
            }
        }
    }
    return vars_;
}


const std::vector< attribute >&
ncfile::atts() const
{
    if ( atts_.empty() ) {
        for ( int attid = 0; attid < ngatts_; ++attid ) {
            if ( auto att = inq_att( NC_GLOBAL, attid ) ) {
                atts_.emplace_back( *att );
            }
        }
    }
    return atts_;
}

std::vector< attribute >
ncfile::atts( const variable& var ) const
{
    if ( const int natts = std::get< variable::natts >( var.value() ) ) {
        std::vector< attribute > atts;
        for ( int attid = 0; attid < natts; ++attid ) {
            if ( auto att = inq_att( std::get< variable::varid >( var.value() ), attid ) )
                atts.emplace_back( *att );
        }
        return atts;
    }
    return {};
}

dimension
ncfile::dim( const variable& var ) const
{
    // auto dimid = std::get< variable::dimids >( var.value() );
    // if ( ( dims().size() > dimid ) &&
    //      ( std::get< dimension::dimid >( dims().at( dimid ).value() ) == dimid ) )
    //     return dims().at( dimid );
    return {};
}


std::optional< dimension >
ncfile::inq_dim( int dimid ) const
{
    std::array< char, NC_MAX_NAME > name;
    size_t len(0);
    if ( nc_inq_dim( ncid_, dimid, name.data(), &len ) == NC_NOERR ) {
        return {{ dimid, name.data(), len }};
    }
    return {};
}

std::optional< attribute >
ncfile::inq_att( int varid, int attid ) const
{
    std::array< char, NC_MAX_NAME > name;
    if ( nc_inq_attname( ncid_, varid, attid, name.data() ) == NC_NOERR ) {
        nc_type xtype(0);
        size_t len(0);
        if ( nc_inq_att( ncid_, varid, name.data(), &xtype, &len ) == NC_NOERR ) {
            return {{ varid, attid, name.data(), xtype, len }};
        }
    }
    return {};
}

std::optional< variable >
ncfile::inq_var( int varid ) const
{
    nc_type xtype(0);
    int ndims(0), dimids(0), natts(0);
    std::array< char, NC_MAX_NAME > name;
    if ( nc_inq_varndims( ncid_, varid, &ndims ) == NC_NOERR ) {
        std::vector< int > dims( ndims );
        if ( nc_inq_var( ncid_, varid, name.data(), &xtype, 0, dims.data(), &natts )  == NC_NOERR ) {
            // ADDEBUG() << ">>>>>>> " << std::make_tuple( varid, name.data(), "xtype: ", xtype, "ndims: ", ndims, "natts", natts );
            // for ( size_t i = 0; i < ndims; ++i ) {
            //     ADDEBUG() << "\t\tdims[" << i << "]=" << dims[i];
            // }
            return {{ varid, name.data(), xtype, ndims, std::move( dims ), natts }};
        }
    }
    return {};
}

////////////////
datum_variant_t
ncfile::get_att( const attribute& t ) const
{
    auto [varid, attid, name, xtype, len ] = t.value();
    auto typ = to_variant< nc_types_t >{}( xtype );

    auto is_ok = std::visit( [&]( auto&& x )->datum_variant_t{
        using T = std::decay_t<decltype(x._)>;
        if constexpr ( std::is_same_v<T, char >)
            return get_att_text( t );
        else if constexpr ( std::is_same_v<T, int >)
            return get_att( int{}, t );
        return {};
    }, typ );


    return {};
}

std::string
ncfile::get_att_text( const attribute& t ) const
{
    auto [varid, attid, name, xtype, len ] = t.value();
    auto typ = to_variant< nc_types_t >{}( xtype );
    auto is_ok = std::visit( []( auto&& t )->bool{
        using T = std::decay_t<decltype(t._)>;
        return std::is_same_v<T, char >;
    }, typ );

    if ( is_ok ) {
        size_t size = std::get< attribute::len >( t.value() );
        std::string datum( size + 1, '\0' );
        if ( nc_get_att_text( ncid_, varid, name.c_str(), datum.data() ) == NC_NOERR )
            return datum;
    }

    return {};
}

template<>
std::vector< int >
ncfile::get_att( int, const attribute& t ) const
{
    return {};
}
