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
#include <iostream>
#include <netcdf.h>

void
do_ncdump(int ncid, const char *path)
{
    ADDEBUG() << "## " << __FUNCTION__ << " ## " << path;

    std::array< int, 6 * 12 > data_in;
    int varid(0);
    int ndims(0), nvars(0), ngatts(0), unlimdimid(0);

    if ( nc_inq( ncid, &ndims, &nvars, &ngatts, &unlimdimid ) == NC_NOERR ) {

        ADDEBUG() << std::make_tuple( "ndims", ndims, "nvars", nvars, "ngatts", ngatts, "unlimid", unlimdimid );
        ADDEBUG() << "--------- dimensions:";
        std::array< char, NC_MAX_NAME > name;
        for ( int dimid = 0; dimid < ndims; ++dimid ) {
            size_t len(0);
            if ( nc_inq_dim( ncid, dimid, name.data(), &len ) == NC_NOERR )
                ADDEBUG() << "\t" << std::make_tuple( dimid, name.data(), len );
        }
        ADDEBUG() << "<-------- end dimensions.";

        ADDEBUG() << "--------- variables:";
        for ( int varid = 0; varid < nvars; ++varid ) {
            nc_type xtype;
            int ndims, dimids, natts;
            if ( nc_inq_var( ncid, varid, name.data(), &xtype, &ndims, &dimids, &natts ) == NC_NOERR ) {
                ADDEBUG() << "\t" <<
                    std::make_tuple( varid, name.data(), "xtype", xtype, "ndims", ndims, "dimids", dimids, "natts", natts );
                if ( natts ) {
                    std::array< char, NC_MAX_NAME > attname;
                    for ( int attid = 0; attid < natts; ++attid ) {
                        if ( nc_inq_attname( ncid, varid, attid, attname.data() ) == NC_NOERR ) {
                            ADDEBUG() << "\t\t" << std::make_tuple( varid, attid, attname.data() );
                        }
                    }

                }
            }
        }
        ADDEBUG() << "<--------- end variables.";

        ADDEBUG() << "--------- attributes:";
        int natts(0);
        if ( nc_inq_natts( ncid, &natts ) == NC_NOERR ) {

            for ( int attid = 0; attid < natts; ++attid ) {
                if ( nc_inq_attname( ncid, NC_GLOBAL, attid, name.data() ) == NC_NOERR ) {
                    nc_type xtype;
                    size_t len;
                    if ( nc_inq_att( ncid, NC_GLOBAL, name.data(), &xtype, &len ) == NC_NOERR ) {
                        ADDEBUG() << std::make_tuple( attid, name.data(), "xtype", xtype, "len", len);
                    }
                }
            }
        }
        ADDEBUG() << "<--------- end attributes.";

        if ( nc_inq_varid(ncid, "data", &varid ) == NC_NOERR ) {
            ADDEBUG() << "data: " << varid;
        }

    }
}

void
do_ncdump( const adnetcdf::netcdf::ncfile& file )
{
    ADDEBUG() << "dimensions:";
    for ( const auto& dim: file.dims() )
        ADDEBUG() << "\t" << dim.value();

    ADDEBUG() << "variables:";
    for ( const auto& var: file.vars() )
        ADDEBUG() << "\t" << var.value();

    ADDEBUG() << "attributes:";
    for ( const auto& att: file.atts() )
        ADDEBUG() << "\t" << att.value();

}


void open_netcdf( const char * filename )
{
    ADDEBUG() << "netcdf library version: " << nc_inq_libvers();
    if ( auto file = adnetcdf::netcdf::open( boost::filesystem::path( filename ) ) ) {
        ADDEBUG() << file.path() << " open success.";
        ADDEBUG() << "file.kind: " << file.kind() << file.kind_extended();
        //do_ncdump( file.ncid(), filename );
        do_ncdump( file );
    } else {
        ADDEBUG() << file.path() << " open failed.";
    }
}


int
main( int argc, const char * argv[] )
{
    if ( --argc ) {
        ++argv;
        while ( argc-- )
            open_netcdf( *argv++ );
        return 0;
    }
    ADDEBUG() << "No argument specified.";
    open_netcdf( "/Users/toshi/data/SFC/2023-0901/COR40_1.CDF" );
    //open_netcdf( "/Users/toshi/src/build-Darwin-arm64/netcdf-c-4.9.2/examples/C/simple_xy.nc" );
    return 0;
}
