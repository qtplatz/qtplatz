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

//#include <QtCore/QCoreApplication>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "mcast_sender.hpp"
#include "dgram_server.hpp"

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    using boost::asio::ip::udp;

    boost::asio::io_service io_service;

    mcast_sender s( io_service, boost::asio::ip::address::from_string( "224.9.9.2" ) );
    dgram_server d( io_service );
    io_service.run();

/*
    exit(0);

    udp::endpoint bcast_endpoint( boost::asio::ip::address_v4::any(), 7000 );
    udp::socket socket( io_service, bcast_endpoint );
    socket.set_option( boost::asio::socket_base::broadcast( true ) );

    std::cout << "listening on: " 
              << bcast_endpoint.address().to_string() 
              << "/" << bcast_endpoint.port()
              << std::endl;

    for ( ;; ) {

        boost::array<char, 1> recv_buf;
        udp::endpoint remote_endpoint;
        boost::system::error_code error;

        socket.receive_from( boost::asio::buffer( recv_buf ), remote_endpoint, 0, error );
        
        if ( error && error != boost::asio::error::message_size )
            throw boost::system::system_error( error );

        std::cout << "requested from: " 
                  << remote_endpoint.address().to_string() 
                  << "/" << remote_endpoint.port()
                  << std::endl;

        boost::posix_time::ptime pt( boost::posix_time::second_clock::local_time() );
        std::string message = boost::posix_time::to_simple_string( pt );        
        boost::system::error_code ignored_error;
        socket.send_to( boost::asio::buffer( message), remote_endpoint, 0, ignored_error );

    }
*/
    return 0;
}
