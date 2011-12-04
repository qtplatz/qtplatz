/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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

#include "mcast_receiver.hpp"
#include "dgram_server.hpp"
#include "lifecycle.hpp"

#include <QtCore/QCoreApplication>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>

class dgram_state_machine : public dgram_server, public lifecycle {
public:
    dgram_state_machine( boost::asio::io_service& io ) : dgram_server( io ) {
    }

    bool operator()( const boost::asio::ip::udp::endpoint&, const char *, std::size_t );
};

bool
dgram_state_machine::operator()( const boost::asio::ip::udp::endpoint& endpoint, const char *, std::size_t )
{
    std::cout << "dgram_state_machine..." << std::endl;
    
    boost::array<char, 1> send_buf = {{ 0 }};
    socket_.send_to( boost::asio::buffer( send_buf ), endpoint );

    std::cout << "dgram sent to: " << endpoint.address().to_string() << "/" << endpoint.port() << std::endl;

    return true;
}


int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    using boost::asio::ip::udp;
    
    boost::asio::io_service io_service;

    lifecycle lifecycle;
    mcast_receiver mcast( io_service
                          , lifecycle
                          , boost::asio::ip::address::from_string( "0.0.0.0" )
                          , boost::asio::ip::address::from_string( "224.9.9.2" ) );
    dgram_state_machine dgram( io_service );
    lifecycle.register_client( &dgram );

    io_service.run();
}
