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
#include <boost/json.hpp>
#include <iomanip>

void
do_ncdump( const adnetcdf::netcdf::ncfile& file )
{
    namespace nc = adnetcdf::netcdf;

    std::cout << "dimensions:" << std::endl;
    for ( const auto& dim: file.dims() ) {
        auto [dimid,name,len] = dim.value();
        std::cout << "\t" << std::setw(24) << name << "\t=\t" << len << std::endl; // boost::json::value_from( dim );
    }

    std::cout << "variables:" << std::endl;
    for ( const auto& var: file.vars() ) {
        auto [varid,name,type,ndims,natts] = var.value();
        auto type_name = nc::nc_type_name( type, nc::nc_types_t{} );
        auto dimentions = file.dims( var );

        std::ostringstream o;
        if ( not dimentions.empty() ) {
            o << "\t[";
            size_t count = 0;
            for ( const auto& d: dimentions ) {
                o << (count++ ? "," : "" ) << d.len();
            }
            o << "]";
        }
        std::cout << "\t" << std::setw(28) << name
                  << o.str()
                  << std::endl;

        using namespace adnetcdf::netcdf;
        for ( const auto& att: file.atts( var ) ) {
            auto [varid,attid,name,type,len] = att.value();
            std::cout << "\t\t:" << name << "\t" << file.get_att_text( att )
                      << std::endl;
        }
    }

    std::cout << "attributes:" << std::endl;
    for ( const auto& att: file.atts() ) {
        auto [varid,attid,name,type,len] = att.value();
        std::cout << "\t\t:" << std::setw(32) << std::left << name << "\t\"" << file.get_att_text( att ) << "\";"
                  << std::endl;
    }
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
