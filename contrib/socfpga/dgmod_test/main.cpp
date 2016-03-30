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

#include <dgmod/dgmod_delay_pulse.h>
#include <boost/format.hpp>
#include <cstdio>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

dgmod_protocol_sequence __protocols__;

int
main( int argc, char* argv[] )
{
    --argc;
    ++argv;
    
    if ( argc && std::strcmp( argv[ 0 ], "-w" ) == 0 ) {
        dgmod_protocol_sequence s;
        s.size_ = 4;
        int pidx(0);
        for ( auto& proto: s.protocols_ ) {
            proto.replicates_ = ++pidx;
            int ch( 1 );
            for ( auto& pulse: proto.delay_pulses_ ) {
                pulse.delay_ = pidx * 100 + ch * 10;
                pulse.width_ = ch * 10;
                ++ch;
            }
        }
        int fd = open( "/dev/dgmod0", O_WRONLY );
        if ( fd > 0 ) {
            std::cout << "Wrote " << write( fd, &s, sizeof( s ) ) << " octets" << std::endl;
        }
    } else {
    
        int fd = open( "/dev/dgmod0", O_RDONLY );
        if ( fd > 0 ) {
            std::cout << read( fd, &__protocols__, sizeof( __protocols__ ) ) << std::endl;
            std::cout << "Has " << __protocols__.size_ << " protocols" << std::endl;
            int pidx(0);
            for ( const auto& proto: __protocols__.protocols_ ) {
                std::cout << "Protocol[" << pidx++ << "]" << std::endl;
                std::cout << "\treplicates: " << proto.replicates_ << std::endl;
                int ch(0);
                for ( const auto& pulse: proto.delay_pulses_ ) {
                    std::cout << boost::format( "\t[CH-%d]\t0x%08x, 0x%08x" )
                        % ch++ % pulse.delay_ % pulse.width_ << std::endl;
                }
            }
            
        } else {
            std::perror( "open failed" );
        }
    }
}
