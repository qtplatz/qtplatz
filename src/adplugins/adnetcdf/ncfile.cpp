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
{
}

ncfile::ncfile( const std::filesystem::path& path, open_mode mode ) : rcode_( -1 )
                                                                    , ncid_( 0 )
                                                                    , path_( path )
{
    rcode_ = nc_open( path.string().c_str(), int( mode), &ncid_ );

    int formatn, nc_extended, nc_mode;
    nc_inq_format( ncid_, &formatn);
    nc_inq_format_extended( ncid_, &nc_extended, &nc_mode );

    ADDEBUG() << "formatn, extended, nc_mode: " << std::make_tuple( formatn, nc_extended, nc_mode );

}

int32_t ncfile::rcode() const { return rcode_; }
int32_t ncfile::ncid() const  { return ncid_; }
const std::filesystem::path& ncfile::path() const { return path_; }

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

    int nc_kind;
    if ( nc_inq_format( ncid_, &nc_kind ) == NC_NOERR ) {
        auto it = std::find_if( list, list + sizeof(list)/sizeof(list[0]), [&](const auto& a){ return std::get<0>(a) == nc_kind; } );
        if ( it != list + sizeof(list)/sizeof(list[0]) )
            return *it;
        return { nc_kind, "unrecognized file format" };
    }
    return {};
}

std::tuple< int, uint16_t, std::string >
ncfile::kind_extended() const
{
    int nc_extended, nc_mode;
    int nc_kind;
    if ( nc_inq_format( ncid_, &nc_kind ) == NC_NOERR
         && nc_inq_format_extended( ncid_, &nc_extended, &nc_mode ) == NC_NOERR ) {

        switch ( nc_kind ) {
        case NC_FORMATX_NC3:
            if( nc_mode & NC_CDF5)
                return {nc_kind, nc_mode, "64-bit data"};
            else if( nc_mode & NC_64BIT_OFFSET)
                return {nc_kind, nc_mode, "64-bit offset"};
            else
                return {nc_kind, nc_mode, "classic"};
        case NC_FORMATX_NC_HDF5:
            return { nc_kind, nc_mode, "HDF5" };
        case NC_FORMATX_NC_HDF4:
            return {nc_kind, nc_mode, "HDF4" };
        case NC_FORMATX_PNETCDF:
            return {nc_kind, nc_mode, "PNETCDF" };
        case NC_FORMATX_DAP2:
            return {nc_kind, nc_mode, "DAP2" };
        case NC_FORMATX_DAP4:
            return {nc_kind, nc_mode, "DAP4" };
        case NC_FORMATX_UNDEFINED:
            return {nc_kind, nc_mode, "unknown" };
        default:
            return {nc_kind, nc_mode, "unrecognized"};
        }
    }
    return {};
}

const std::vector< dimension >&
ncfile::dims() const
{
    if ( dims_.empty() ) {
        int ndims, nvars, ngatts, unlimdimid;
        if ( nc_inq( ncid_, &ndims, &nvars, &ngatts, &unlimdimid ) == NC_NOERR ) {
            for ( int dimid = 0; dimid < ndims; ++dimid ) {
                std::array< char, NC_MAX_NAME > name;
                size_t len(0);
                if ( nc_inq_dim( ncid_, dimid, name.data(), &len ) == NC_NOERR ) {
                    dimension dim( dimid, name.data(), len );
                    dims_.emplace_back( dim );
                }
            }
        }
    }
    return dims_;
}

const std::vector< variable >&
ncfile::vars() const
{
    if ( vars_.empty() ) {
        int ndims(0), nvars(0), ngatts(0), unlimdimid(0);
        if ( nc_inq( ncid_, &ndims, &nvars, &ngatts, &unlimdimid ) == NC_NOERR ) {
            for ( int varid = 0; varid < nvars; ++varid ) {
                nc_type xtype;
                int ndims, dimids, natts;
                std::array< char, NC_MAX_NAME > name;
                if ( nc_inq_var( ncid_, varid, name.data(), &xtype, &ndims, &dimids, &natts ) == NC_NOERR ) {
                    vars_.emplace_back( variable{ varid, name.data(), xtype, ndims, dimids, natts } );
                }
            }
        }
    }
    return vars_;
}


const std::vector< attribute >&
ncfile::atts() const
{
    if ( atts_.empty() ) {
        int natts(0);
        if ( nc_inq_natts( ncid_, &natts ) == NC_NOERR ) {

            for ( int attid = 0; attid < natts; ++attid ) {
                std::array< char, NC_MAX_NAME > name;
                if ( nc_inq_attname( ncid_, NC_GLOBAL, attid, name.data() ) == NC_NOERR ) {
                    nc_type xtype;
                    size_t len;
                    if ( nc_inq_att( ncid_, NC_GLOBAL, name.data(), &xtype, &len ) == NC_NOERR ) {
                        ADDEBUG() << std::make_tuple( attid, name.data(), "xtype", xtype, "len", len);

                        //atts_.emplace_back( attribute( NC_GLOBAL, attid, name.data(), xtype, len ) );
                    }
                }
            }
        }
    }

    return atts_;
}
