// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include <dgmod/hps.h>
#include <boost/format.hpp>
#include <cstdio>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/mman.h>

enum { gpio_map_size = 4096 };
    
int
main( int argc, char* argv[] )
{
    --argc;
    ++argv;

    std::cout << "Hello" << std::endl;

    int fd = open( "/dev/mem", (O_RDWR | O_SYNC) );
    if ( fd < 0 ) {
        std::cerr << "Error: /dev/mem could not be opened" << std::endl;
        return 1;
    }

    auto mapped_ptr = reinterpret_cast< uint32_t * >(mmap( 0, gpio_map_size
                                                           , (PROT_READ | PROT_WRITE), MAP_SHARED, fd, gpio1_addr ));
    
    if ( mapped_ptr == MAP_FAILED ) {
        std::cerr << "Error: mmap() failed" << std::endl;
        return 1;
    }

    size_t offs = 0; //alt_gpio1_ofst / sizeof( uint32_t );

    std::cout << "mmap ok " << boost::format( "%08x" ) % mapped_ptr << std::endl;
    
    mapped_ptr [ offs + 1 ] = 0x01000000; // set direction to output

    mapped_ptr [ offs ] |=  0x01000000; // LED on
    std::cout << boost::format( "led data = 0x%08x" ) % mapped_ptr [ offs ] << std::endl;

    sleep(1);
    mapped_ptr [ offs ] = mapped_ptr [ offs ] & ~0x01000000; // LED on

    std::cout << boost::format( "led data = 0x%08x" ) % mapped_ptr [ offs ] << std::endl;

    std::cout << "Press user key (adjesent to cold reset button on de0-nano-soc)" << std::endl;
    std::cout << "Ctrl-C to exit this program." << std::endl;

    while ( true ) {
        auto scan_input = mapped_ptr[ gpio_ext_porta / sizeof( uint32_t ) ];
        // std::cout << boost::format( " scan_input=%x" ) % ( scan_input & 0x02000000 ) << std::endl;
        if ( ~scan_input & 0x02000000 ) {
            mapped_ptr[ offs ] |= 0x01000000;
        } else {
            mapped_ptr[ offs ] = (~0x01000000 & mapped_ptr[ offs ]);
        }
    }

    munmap( mapped_ptr, gpio_map_size );
    close(fd);

    return 0;
}
