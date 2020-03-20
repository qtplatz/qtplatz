// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2019-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019-2020 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "dataprocessor.hpp"
#include "datareader.hpp"
#include "singleton.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/isotopecluster.hpp>
// #include <adcontrols/datafile.hpp>
// #include <adcontrols/datareader.hpp>
// #include <adportable/debug.hpp>
// #include <adcontrols/datareader.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/molecule.hpp>
#include <adportable/debug.hpp>
#include <adportfolio/portfolio.hpp>
#include <boost/filesystem.hpp>
#include <codecvt>
#include <memory>

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "wstp.h"

using namespace ws_adprocessor;

int
main( int argc, char * argv[] )
{
    ADDEBUG() << "main(" << argc << ", " << argv[0] << ")";
    return WSMain( argc, argv );
}

int
addtwo( int i, int j )
{
    ADDEBUG() << "addtwo(" << i << ", " << j << ")";
	return i+j;
}

int
counter( int i )
{
    static int __counter;
    __counter += i;
    return __counter;
}

double
monoIsotopicMass( const char * formula )
{
    double exactMass = adcontrols::ChemicalFormula().getMonoIsotopicMass( adcontrols::ChemicalFormula::split( formula ) );
    return exactMass;
}

void
isotopeCluster( const char * formula, double resolving_power )
{
    ADDEBUG() << __FUNCTION__ << "(" << formula << ", " << resolving_power << ")";

    adcontrols::MassSpectrum ms;

    adcontrols::isotopeCluster()( ms, formula, 1.0, resolving_power );

    long dimensions[ 2 ] = { long(ms.size()), 2 };
    const char * heads[ 2 ] = { "List", "List" };
    std::unique_ptr< double[] > a( std::make_unique< double[] >( ms.size() * 2 ) );

    for ( size_t i = 0; i < ms.size(); ++i ) {
        a[ i * 2 + 0 ] = ms.mass( i );
        a[ i * 2 + 1 ] = ms.intensity( i );
    }
    WSPutDoubleArray( stdlink, a.get(), dimensions, heads, 2 );
}

int
adFileOpen( const char * name )
{
    int id(-1);

     ADDEBUG() << __FUNCTION__ << "(" << name << ")";

     boost::filesystem::path path( name );
     if ( boost::filesystem::exists( path ) ) {
         if ( auto dp = std::make_shared< dataProcessor >() ) {
             if ( dp->open( path.wstring() ) ) {
                 id = singleton::instance()->set_dataProcessor( dp );
             }
         }
     }
     return id;
}

int
fileClose( int file )
{
    singleton::instance()->remove_dataProcessor( file );
    return 0;
}

void
dataReaders( int file )
{
    if ( auto dp = singleton::instance()->dataProcessor( file ) ) {
        auto size = dp->dataReaders().size();
        
        boost::property_tree::ptree pt;
        boost::property_tree::ptree child;
        
        for ( auto reader : dp->dataReaders() ) {
            boost::property_tree::ptree a;
            a.put( "display_name", reader->display_name() );
            a.put( "objtext", reader->objtext() );
            a.put( "objuuid", reader->objuuid() );
     
            child.push_back( std::make_pair( "", a ) );
        }

        pt.add_child( "dataReader", child );

        std::ostringstream o;
        write_json( o, pt );
        WSPutString( stdlink, o.str().c_str() );
    } else {
        WSPutSymbol(stdlink, "$Failed");
    }
}

int
dataReader( int file, const char * objuuid )
{
    if ( auto dp = singleton::instance()->dataProcessor( file ) ) {
        return dp->dataReader( objuuid );
    }
    return (-1);
}

void
adProcessed( int id )
{
    if ( auto dp = singleton::instance()->dataProcessor( id ) ) {
        auto portfolio = dp->portfolio();
        WSPutString( stdlink, portfolio.xml().c_str() );
    } else {
        WSPutSymbol(stdlink, "$Failed");
    }
}

void
readSpectrum( int id, int readerId )
{
    if ( auto dp = singleton::instance()->dataProcessor( id ) ) {
        if ( auto ms = dp->readMassSpectrum( readerId ) ) {
            std::vector< double > a;
            for ( auto pms: adcontrols::segment_wrapper<>( *ms ) ) {
                for ( size_t i = 0; i < pms.size(); ++i ) {
                    a.emplace_back( pms.mass( i ) );
                    a.emplace_back( pms.time( i ) );
                    a.emplace_back( pms.intensity( i ) );
                }
            }
            long dimensions[ 2 ] = { long(a.size() / 3), 3 };
            const char * heads[ 3 ] = { "List", "List", "List" };            
            WSPutDoubleArray( stdlink, a.data(), dimensions, heads, 2 );
            return;
        }
    }
    WSPutSymbol(stdlink, "$Failed");
}

int
next( int id, int readerId )
{
    if ( auto dp = singleton::instance()->dataProcessor( id ) ) {
        return dp->next( readerId );

    }
    return (-1);
}
