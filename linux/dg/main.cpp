/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "ppio.hpp"
#include <boost/program_options.hpp>
#include <chrono>
#include <iostream>
#include <thread>

namespace po = boost::program_options;

int
main( int argc, char * argv [] )
{
    po::variables_map vm;
    po::options_description description( "test_u5303a" );
    {
        description.add_options()
            ( "help,h",    "Display this help message" )
            ( "interval,i",    po::value<int>()->default_value( 1000 ), "Trig interval (us)" )
            ( "delay",         po::value<int>()->default_value( 2 ), "Pulse delay (us)" )
            ( "width",         po::value<int>()->default_value( 1 ), "Pulse delay (us)" )
            ;
        po::store( po::command_line_parser( argc, argv ).options( description ).run(), vm );
        po::notify(vm);
    }
    
    if ( vm.count( "help" ) ) {
        std::cout << description;
        return 0;
    }

    int trig_interval = vm[ "interval" ].as<int>();
    int pulse_delay = vm[ "delay" ].as<int>();
    int pulse_width = vm[ "width" ].as<int>();

    ppio pp;

    for ( ;; ) {
        pp << uint8_t(0xf0); // H        
        auto tp0 = std::chrono::steady_clock::now();
        auto tp_next        = tp0 + std::chrono::microseconds( trig_interval );
        auto tp_trig_down   = tp0 + std::chrono::microseconds( 1 );
        auto tp_pulse_raise = tp0 + std::chrono::microseconds( pulse_delay );
        auto tp_pulse_down  = tp_pulse_raise + std::chrono::microseconds( pulse_width );

        std::this_thread::sleep_until( tp_trig_down ); // trig width
        pp << uint8_t(0x00); // L
        
        std::this_thread::sleep_until( tp_pulse_raise );
        pp << uint8_t(0x0f);

        std::this_thread::sleep_until( tp_pulse_down );
        pp << uint8_t(0x00);
        
        std::this_thread::sleep_until( tp_next );
    }
    return 0;
}
