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
#include "nc_type.hpp"
#include "dimension.hpp"
#include "variable.hpp"
#include "attribute.hpp"
#include <adportable/debug.hpp>
#include <netcdf.h>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <numeric>
#include <iostream>
#include <iomanip> // quoted

#if 0
extern "C" {
    int nc_close( int ) { return 0; }
    int nc_open( const char *, int, int * ) { return 0; }
    int nc_inq( int, int *, int *, int *, int * ) { return 0; }
    int nc_inq_att( int, int, const char *, nc_type *, size_t * ) { return 0; }
    int nc_inq_dim( int, int, char *, size_t *) { return 0; }
    int nc_inq_format( int, int * )  { return 0; }
    int nc_inq_format_extended( int, int *, int * ) { return 0; }
    int nc_inq_attname( int, int, int, char * ) { return 0; }
    int nc_inq_var( int, int, char *, nc_type *, int *, int *, int * ) { return 0; }
    int nc_inq_varndims( int, int, int * ) { return 0; }
    int nc_get_att_int(int, int, const char *, int *) { return 0; }
    int nc_get_att_text(int, int, const char *, char *) { return 0; }
    int nc_get_var_schar( int, int, int8_t *) { return 0; }
    int nc_get_var_ubyte( int, int, uint8_t *) { return 0; }
    int nc_get_var_short( int, int, int16_t *) { return 0; }
    int nc_get_var_ushort( int, int, uint16_t *) { return 0; }
    int nc_get_var_int( int, int, int *) { return 0; }
    int nc_get_var_uint( int, int, uint32_t *) { return 0; }
    int nc_get_var_longlong( int, int, long long *) { return 0; }
    int nc_get_var_ulonglong( int, int, uint64_t *) { return 0; }
    int nc_get_var_float( int, int, float *) { return 0; }
    int nc_get_var_double( int, int, double *) { return 0; }
    int nc_get_vara( int, int, const size_t *, const size_t *, void *) { return 0; }
}
#endif

namespace adnetcdf {
    namespace netcdf {
        ncfile
        open( const std::filesystem::path& path, open_mode mode )
        {
            return ncfile{ path, mode };
        }
    }
}

namespace {
    // helper type for the visitor #4
    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };
    // explicit deduction guide (not needed as of C++20)
    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;
}

namespace adnetcdf { namespace netcdf {

        struct nc_helper {
            std::vector< size_t > operator()( const ncfile& file, const variable& var ) const {
                std::vector< size_t > dsize;
                auto dimentions = file.dims( var );
                auto [varid,name,type,ndims,natts] = var.value();
                std::transform( dimentions.begin(), dimentions.end(), std::back_inserter( dsize ), [](const auto& a){ return a.len(); });
                return dsize;
            }
        };

        //------------------------ get_var ---------------------
        template< typename T > int nc_get_var_( T tag, int ncid, int varid, T * p );

        template<> int nc_get_var_<int8_t>( int8_t, int ncid, int varid, int8_t * p ) { return nc_get_var_schar( ncid, varid, p ); }
        template<> int nc_get_var_<int16_t>( int16_t, int ncid, int varid, int16_t * p ) { return nc_get_var_short( ncid, varid, p ); }
        template<> int nc_get_var_<int32_t>( int32_t, int ncid, int varid, int32_t * p ) { return nc_get_var_int( ncid, varid, p ); }
        template<> int nc_get_var_<int64_t>( int64_t, int ncid, int varid, int64_t * p ) {
            long long t;
            int rcode = nc_get_var_longlong( ncid, varid, &t );
            *p = t;
            return rcode;
        }

        template<> int nc_get_var_<float>( float, int ncid, int varid, float * p ) { return nc_get_var_float( ncid, varid, p ); }
        template<> int nc_get_var_<double>( double, int ncid, int varid, double * p ) { return nc_get_var_double( ncid, varid, p ); }

        template<> int nc_get_var_<uint8_t>( uint8_t, int ncid, int varid, uint8_t * p ) { return nc_get_var_ubyte( ncid, varid, p ); }
        template<> int nc_get_var_<uint16_t>( uint16_t, int ncid, int varid, uint16_t * p ) { return nc_get_var_ushort( ncid, varid, p ); }
        template<> int nc_get_var_<uint32_t>( uint32_t, int ncid, int varid, uint32_t * p ) { return nc_get_var_uint( ncid, varid, p ); }
        template<> int nc_get_var_<uint64_t>( uint64_t, int ncid, int varid, uint64_t * p ) {
            unsigned long long t;
            int rcode = nc_get_var_ulonglong( ncid, varid, &t );
            *p = t;
            return rcode;
        }

        //------------------------ get_att -----------------------
        template< typename T > int nc_get_att_( T tag, int ncid, int varid, const char * name, T * p );

        template<> int nc_get_att_< char >( char, int ncid, int varid, const char * name, char * p )
        {
            return nc_get_att_text( ncid, varid, name, p );
        }
        template<> int nc_get_att_< int32_t >( int32_t, int ncid, int varid, const char * name, int32_t * p )
        {
            return nc_get_att_int( ncid, varid, name, p );
        }

        //------------------------
        struct nc_var_reader {
            const ncfile& ncfile_;
            nc_var_reader( const ncfile& file ) : ncfile_( file ) {}

            // read into vector
            template< typename T > std::vector< T > operator()( T tag, const variable& var ) const {
                auto dsize = nc_helper()( ncfile_, var );
                size_t len = std::accumulate( dsize.begin(), dsize.end(), 1, std::multiplies<size_t>() );
                std::vector< T > data(len);
                if ( nc_get_var_( tag, ncfile_.ncid(), var.varid(), data.data() ) == NC_NOERR )
                    return data;
                return {};
            }

            // string reader
            std::string operator()( std::string tag, const variable& var ) const {
                nc_helper()( ncfile_, var );
                return std::string{};
            }

            std::vector< std::string > operator()( std::vector< std::string > tag, const variable& var ) const {
                auto dsize = nc_helper()( ncfile_, var );
                size_t len = std::accumulate( dsize.begin(), dsize.end(), 1, std::multiplies<size_t>() );
                std::vector< char > data( len );
                std::vector< size_t > col(dsize.size() + 1), edg(dsize.size() + 1);
                col[0] = 0;
                col[1] = 0;
                edg[0] = 1;
                edg[1] = dsize[1];
                col[dsize.size()] = edg[dsize.size()] = 0;

                std::vector< std::string > datum;
                for ( size_t i = 0; i < dsize[0]; ++i ) {
                    col[0] = i;
                    if ( nc_get_vara( ncfile_.ncid(), var.varid(), col.data(), edg.data(), data.data() ) == NC_NOERR ) {
                        datum.emplace_back( data.data() );
                    } else {
                        datum.emplace_back( std::string{} );
                    }
                }
                return datum;
            }
        };

        //--------------------------
        //------------------------
        struct nc_att_reader {
            const ncfile& ncfile_;
            nc_att_reader( const ncfile& file ) : ncfile_( file ) {}

            std::string operator()( std::string tag, const attribute& att ) const {
                std::string data( att.len(), '\0' );
                if ( nc_get_att_text( ncfile_.ncid(), att.varid(), att.name(), data.data() ) == NC_NOERR )
                    return std::string{ data.c_str() }; // remove trailing zero
                return {};
            }
            template< typename T > std::vector< T >
            operator()( T tag, const attribute& att ) const {
                std::vector< T > data( att.len() );
                if ( nc_get_att_<T>( T{}, ncfile_.ncid(), att.varid(), att.name(), data.data() ) == NC_NOERR )
                    return data;
                return {};
            }
        };
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

    for ( int dimid = 0; dimid < ndims_; ++dimid ) {
        if ( auto dim = inq_dim( dimid ) )
            dims_.emplace_back( *dim );
    }

    for ( int attid = 0; attid < ngatts_; ++attid ) {
        if ( auto att = inq_att( NC_GLOBAL, attid ) ) {
            atts_.emplace_back( *att );
        }
    }
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
    return atts_;
}

std::vector< attribute >
ncfile::atts( const variable& var ) const
{
    auto [varid,name,type,ndims,natts] = var.value();
    if ( natts ) {
        std::vector< attribute > atts;
        for ( int attid = 0; attid < natts; ++attid ) {
            if ( auto att = inq_att( varid, attid ) )
                atts.emplace_back( *att );
        }
        return atts;
    }
    return {};
}

std::vector< dimension >
ncfile::dims( const variable& var ) const
{
    std::vector< dimension > dims;
    for ( const auto& id: var.dimids() ) {
        if ( dims_.size() > id )
            dims.emplace_back( dims_.at( id ) );
    }
    return dims;
}


std::optional< dimension >
ncfile::inq_dim( int dimid ) const
{
    std::array< char, NC_MAX_NAME > name = {};
    size_t len(0);
    if ( nc_inq_dim( ncid_, dimid, name.data(), &len ) == NC_NOERR ) {
        return {{ dimid, name.data(), len }};
    }
    return {};
}

std::optional< attribute >
ncfile::inq_att( int varid, int attid ) const
{
    std::array< char, NC_MAX_NAME > name = {};
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
    std::array< char, NC_MAX_NAME > name = {};
    if ( nc_inq_varndims( ncid_, varid, &ndims ) == NC_NOERR ) {
        std::vector< int > dims( ndims );
        if ( nc_inq_var( ncid_, varid, name.data(), &xtype, 0, dims.data(), &natts )  == NC_NOERR ) {
            return {{ varid, name.data(), xtype, ndims, std::move( dims ), natts }};
        }
    }
    return {};
}

////////////////////////////////////////////////

datum_t
ncfile::readData( const attribute& t ) const
{
    auto [varid, attid, name, xtype, len ] = t.value();
    auto typ = to_variant< nc_types_t >{}( xtype );

    auto datum = std::visit( [&]( auto&& x )->datum_t{
        using T = std::decay_t<decltype(x._)>;
        if constexpr ( std::is_same_v<T, char >)
            return nc_att_reader(*this)( std::string(), t );
        else if constexpr ( std::is_same_v<T, int >)
            return nc_att_reader(*this)( int(), t );
        return {};
    }, typ );

    return datum;
}

////////////////////////////////////////////////

datum_t
ncfile::readData( const variable& var ) const
{
    auto dimensions = dims( var );
    auto typ = to_variant< nc_types_t >()( std::get< variable::_type >( var.value() ) );

    nc_var_reader reader(*this);
    auto ovld = overloaded{
        [&]( const nc_type_t< NC_NAT>& x )->datum_t   { ADDEBUG() << "unhandled"; return {}; },
        [&]( const nc_type_t< NC_BYTE>& x )->datum_t  { ADDEBUG() << "unhandled"; return {}; },
        [&]( const nc_type_t< NC_CHAR>& x )->datum_t  { return reader( std::vector< std::string >{}, var); },
        //[&]( const nc_type_t< NC_SHORT>& x )->datum_t { ADDEBUG() << "unhandled"; return {}; }, // handle by auto type bellow
        //[&]( const nc_type_t< NC_INT>& x )->datum_t   { ADDEBUG() << "unhandled"; return {}; }, // handle by auto type bellow
        //[&]( const nc_type_t< NC_FLOAT>& x )->datum_t { ADDEBUG() << "unhandled"; return {}; }, // handle by auto type
        //[&]( const nc_type_t< NC_DOUBLE>& x )->datum_t{ ADDEBUG() << "unhandled"; return {}; }, // handle by auto type
        [&]( const nc_type_t< NC_UBYTE>& x )->datum_t { ADDEBUG() << "unhandled"; return {}; },
        [&]( const nc_type_t< NC_USHORT>& x )->datum_t{ ADDEBUG() << "unhandled"; return {}; },
        [&]( const nc_type_t< NC_UINT>& x )->datum_t  { ADDEBUG() << "unhandled"; return {}; },
        [&]( const nc_type_t< NC_INT64>& x )->datum_t { ADDEBUG() << "unhandled"; return {}; },
        [&]( const nc_type_t< NC_UINT64>& x )->datum_t{ ADDEBUG() << "unhandled"; return {}; },
        [&]( const nc_type_t< NC_STRING>& x )->datum_t{ ADDEBUG() << "unhandled"; return {}; },
        [&]( const auto& x )->datum_t{ return reader( x._, var); },
    };
    return std::visit( ovld, typ );
}
