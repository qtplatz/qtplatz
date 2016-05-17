/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "pio.hpp"
#include <boost/program_options.hpp>
#include <chrono>
#include <iostream>
#include <thread>

namespace po = boost::program_options;

int
main( int argc, char * argv [] )
{
    po::variables_map vm;
    po::options_description description( "dgtool" );
    {
        description.add_options()
            ( "help,h",      "Display this help message" )
            ( "interval,i",  po::value< int >()->default_value( 1000 ), "read interval (ms)" )
            ( "args",        po::value< std::vector< std::string > >(), "arg ..." );
            ;
        po::store( po::command_line_parser( argc, argv ).options( description ).run(), vm );
        po::notify(vm);

        po::positional_options_description p;
        p.add( "args",  -1 );
        po::store( po::command_line_parser( argc, argv ).options( description ).positional(p).run(), vm );
        po::notify(vm);
    }
    
    if ( vm.count( "help" ) ) {
        std::cout << description;
        return 0;
    }

    dgpio::pio pio;

    if ( pio.open() ) {

        std::cout << "pio open successed." << std::endl;

        if ( vm.count( "args" ) ) {

            for ( auto& arg: vm[ "args" ].as< std::vector< std::string > >() ) {
                for ( auto& c: arg )
                    pio.set_protocol_number( c );
            }
        } else {

            int milliseconds = vm[ "interval" ].as< int >();
            size_t count = 0;
            while( true ) {
                std::cout << count++ << ": " << pio.protocol_number() << std::endl;
                std::this_thread::sleep_for( std::chrono::milliseconds( milliseconds ) );
            }
        }
    }
    return 0;
}
