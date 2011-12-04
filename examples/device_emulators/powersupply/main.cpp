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
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "mcast_sender.hpp"
#include "dgram_server.hpp"

using boost::asio::ip::udp;

class dgram_state_machine {
public:
    dgram_state_machine( boost::asio::io_service& io ) : io_service_( io )
                                                       , socket_( io, udp::endpoint( udp::v4(), 7000 ) ) {
        start_receive();
    }

    void start_receive() {
        socket_.async_receive_from( boost::asio::buffer( recv_buffer_ )
                                    , remote_endpoint_
                                    , boost::bind( &dgram_state_machine::handle_receive
                                                   , this
                                                   , boost::asio::placeholders::error
                                                   , boost::asio::placeholders::bytes_transferred ) );
    }
    
    void handle_receive( const boost::system::error_code& error, std::size_t ) {
        if ( ! error || error == boost::asio::error::message_size ) {
            // should be 'connect' request
            std::cout << "dgram_state_machine receive from: " 
                      << remote_endpoint_.address().to_string() 
                      << "/" << remote_endpoint_.port()
                      << std::endl;
            sessions_[ remote_endpoint_.port() ] =
                boost::shared_ptr< dgram_server >( new dgram_server( io_service_, remote_endpoint_ ) );
            start_receive();
        }
    }
private:
    boost::asio::io_service& io_service_;
    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    boost::array< char, 1 > recv_buffer_;
    std::map< unsigned short, boost::shared_ptr< dgram_server > > sessions_;
};

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    using boost::asio::ip::udp;

    boost::asio::io_service io_service;

    mcast_sender s( io_service, boost::asio::ip::address::from_string( "224.9.9.2" ) );
    dgram_state_machine hello( io_service );
    //dgram_server hello( io_service );
    io_service.run();

    return 0;
}
