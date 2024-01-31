/**************************************************************************
** Copyright (C) 2010-2024 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2024 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "timesquaredscanlaw.hpp"
#include <adportable/debug.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/scanlaw.hpp>
#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>

namespace po = boost::program_options;

struct arg_to_double {
    std::optional< std::pair< double, std::string > > operator()( const std::string& arg ) {
        char * end {};
        double value = std::strtod( arg.c_str(), &end );
        if ( end != arg.c_str() && *end == '\0' )
            return {{ value, {} }};
        if ( double m = adcontrols::ChemicalFormula().getMonoIsotopicMass( arg ) >= 1.0 ) {
            ADDEBUG() << "------> " << m << " <--" << arg;
            return {{ m, arg }};
        }
        return {};
    }
};

int
main(int argc, char *argv[])
{
    po::variables_map vm;
    po::options_description description( argv[0] );
    {
        description.add_options()
            ( "help,h",      "Display this help message" )
            ( "args",           po::value< std::vector< std::string > >(),             "formulae|times" )
            ( "formulae",       po::value< bool >()->default_value( true ),          "formulae|masses|times" )
            ( "accelerator,v",  po::value< double >()->default_value( 7000 ),        "accelerator voltage (V)" )
            ( "length,L",       po::value< double >()->default_value( 2.0 ),         "flight length (m)" )
            ( "t0",             po::value< double >()->default_value( 0.0 ),         "t0 (s)" )
            ( "query",          po::value< std::string >()->default_value( "time" ), "time|mass|voltage" )
            ;

        po::positional_options_description p;
        p.add( "args",  -1 );
        po::store( po::command_line_parser( argc, argv ).options( description ).positional(p).run(), vm );
        po::notify(vm);
    }

    if ( vm.count( "help" ) || (vm.count( "args" ) == 0) ) {
        std::cout << description;
        return 0;
    }

    if ( vm.count("args") ) {
        for ( const auto& arg: vm[ "args" ].as< std::vector< std::string > >() ) {
            if ( auto kv = arg_to_double()( arg ) ) {
                ADDEBUG() << *kv;
            }
        }
    }


}
