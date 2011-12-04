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

#include "dgram_server.hpp"
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using boost::asio::ip::udp;

dgram_server::dgram_server( boost::asio::io_service& io_service, boost::asio::ip::udp::endpoint& remote )
    : socket_( io_service, udp::endpoint( udp::v4(), 7100 ) )
    , remote_endpoint_( remote )
{
    start_receive();
    sendto( "connect|ack", 12 );
}

void
dgram_server::sendto( const char * data, std::size_t len )
{
    socket_.send_to( boost::asio::buffer( data, len ), remote_endpoint_ );
}

void
dgram_server::start_receive()
{
    socket_.async_receive_from( boost::asio::buffer( recv_buffer_ )
                                , remote_endpoint_
                                , boost::bind( &dgram_server::handle_receive
                                               , this
                                               , boost::asio::placeholders::error
                                               , boost::asio::placeholders::bytes_transferred ) );
}

void
dgram_server::handle_receive( const boost::system::error_code& error, std::size_t )
{
    if ( ! error || error == boost::asio::error::message_size ) {
        
        std::cout << "dgram receive from: " 
                  << remote_endpoint_.address().to_string() 
                  << "/" << remote_endpoint_.port()
                  << std::endl;

        boost::posix_time::ptime pt( boost::posix_time::second_clock::local_time() );
        boost::shared_ptr<std::string> message( new std::string( boost::posix_time::to_simple_string( pt ) ) );
        
        socket_.async_send_to( boost::asio::buffer(*message)
                               , remote_endpoint_
                               , boost::bind( &dgram_server::handle_send
                                              , this
                                              , message
                                              , boost::asio::placeholders::error
                                              , boost::asio::placeholders::bytes_transferred ) );
        start_receive();
    }
}

void
dgram_server::handle_send( boost::shared_ptr<std::string>
                           , const boost::system::error_code&
                           , std::size_t )
{
}
