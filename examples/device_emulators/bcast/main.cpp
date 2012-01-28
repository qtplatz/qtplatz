/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "bcast_server.hpp"
#include "lifecycle.hpp"

#include <QtCore/QCoreApplication>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <set>
#include <iostream>

#define BOOST_LIB_NAME boost_system
#include <boost/config/auto_link.hpp>

static bool connect_on_start = false;
static int port = 7000;
static std::string bcaddr = "192.168.24.255";

using boost::asio::ip::udp;

class bcast_state_machine : public lifecycle {
public:

    bcast_state_machine( udp::socket& s ) : socket_( s ) {
    }

    bool operator()( const boost::asio::ip::udp::endpoint&, const char *, std::size_t );

    bool bcast_connect();

    static const char * frame_read( const char * p, std::string& s )  {
        size_t len = *reinterpret_cast<const boost::uint32_t *>(p);
        p += sizeof( boost::uint32_t );
        while( len-- )
            s += *p++;
        return p;
    }

private:
    udp::socket& socket_;
    std::set< boost::uint16_t > ports_;
};

bool
bcast_state_machine::operator()( const boost::asio::ip::udp::endpoint& endpoint
                                 , const char * p
                                 , std::size_t len )
{
    std::cout << std::dec << "bcast_state_machine lifecycle forward " << len << " bytes from: "
              << endpoint.address().to_string() << "/" << endpoint.port() << std::endl;

    if ( len >= sizeof( LifeCycleFrame ) ) {
        const LifeCycleFrame * pf = reinterpret_cast<const LifeCycleFrame *>(p);
        const char * sp = p + sizeof( LifeCycleFrame );
        while ( size_t( sp - p ) < len ) {
            std::string s;
            sp = frame_read( sp, s );
            std::cout << s << ", ";
        }
        std::cout << std::endl;

        if ( pf->command == HELO && 
             ports_.find( endpoint.port() ) == ports_.end() ) {

            // keep port# in order to avoid duplicate connection
            ports_.insert( endpoint.port() );

            boost::array< char, sizeof(LifeCycleFrame) + 4 > dbuf;
            LifeCycleFrame * ptr = new ( dbuf.data() ) LifeCycleFrame( adportable::protocol::CONN_SYN );
            (void)ptr;

            boost::uint16_t *pseq = reinterpret_cast< boost::uint16_t * >( dbuf.data() + sizeof(LifeCycleFrame) );
            *pseq++ = 0x100;
            *pseq++ = 0; // remote sequence is not known yet

            // Send CONN SYN request to device_ip:port = 7000 (well known #) on power supply module
            boost::asio::ip::udp::endpoint remote_endpoint( endpoint.address(), 7000 );

            // If using broadcast
            // boost::asio::ip::udp::endpoint remote_endpoint( boost::asio::ip::address_v4::any(), 7000 );
            socket_.send_to( boost::asio::buffer( dbuf ), remote_endpoint );
            std::cout << "CONN|SYN sent to: " 
                      << remote_endpoint.address() << "/" << remote_endpoint.port() << std::endl;
        }
        return true;
    }
    return false;
}

bool
bcast_state_machine::bcast_connect()
{
    boost::array< char, sizeof(LifeCycleFrame) + 4 > dbuf;
    new ( dbuf.data() ) LifeCycleFrame( adportable::protocol::CONN_SYN );

    boost::asio::ip::udp::endpoint remote_endpoint( boost::asio::ip::address::from_string( bcaddr ), 7000 );
    // send connect request to 0.0.0.0/7000
    std::cout << "conn|syn request to " << remote_endpoint.address() << "/" << remote_endpoint.port() << std::endl;
    try {
        socket_.send_to( boost::asio::buffer( dbuf ), remote_endpoint );
        return true;
    } catch ( std::exception& ex ) {
        std::cerr << ex.what() << std::endl;
    }
    return false;
}

int main(int argc, char *argv[])
{
    while ( --argc ) {
        ++argv;
        if ( strcmp( *argv, "--connect" ) == 0 ) {
            connect_on_start = true;
            port = 8000;
        } else if ( isdigit( *argv[0] ) ) {
            bcaddr = *argv;
        }
    }
    using boost::asio::ip::udp;
    
    boost::asio::io_service io_service;

    lifecycle lifecycle;

    bcast_server bcast( io_service, port );
    bcast_state_machine machine( bcast.socket() );
    lifecycle.register_client( &machine );

    if ( connect_on_start ) {
        std::cout << "sending connection request" << std::endl;
        machine.bcast_connect();
    }

    io_service.run();
}
