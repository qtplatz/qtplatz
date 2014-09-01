/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "dgram_server.hpp"
#include "lifecycle.hpp"
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using boost::asio::ip::udp;

dgram_server::dgram_server( boost::asio::io_service& io_service )
    : socket_( io_service, udp::endpoint( udp::v4(), 8000 ) )
    , timer_( io_service )
    , local_seq_( 0x100 )
    , remote_seq_( 0 )
{
    start_receive();
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
dgram_server::handle_receive( const boost::system::error_code& error, std::size_t len )
{
    if ( ! error || error == boost::asio::error::message_size ) {

        const size_t fsize = sizeof( LifeCycleFrame );

        const LifeCycleFrame * pf = reinterpret_cast< const LifeCycleFrame *>( recv_buffer_.data() );
        if ( len >= fsize + 2 ) {
            std::cout << "hello/dgram_server::handle_receive recv " << len << " bytes from: " 
                      << remote_endpoint_.address().to_string() 
                      << "/" << std::dec << remote_endpoint_.port()
                      << std::endl;
            const boost::uint16_t * pseq = reinterpret_cast< const boost::uint16_t *>( &recv_buffer_[ fsize ] );
            if ( pf->command == CONN_SYN_ACK ) {
                std::cout << "CONN_SYN|ACK received " << " lseq: " << pseq[0] << " rseq: " << pseq[1] << std::endl;
                timer_.expires_from_now( boost::posix_time::seconds( 3 ) );
                timer_.async_wait( boost::bind( &dgram_server::handle_timeout, this
                                                , boost::asio::placeholders::error ) );
            } else if ( pf->command == DATA_ACK ) {
                std::cout << "DATA|ACK received " << " lseq: " << pseq[0] << " rseq: " << pseq[1] << std::endl;
            } else if ( pf->command == DATA ) {
            }
        }
/*
        boost::posix_time::ptime pt( boost::posix_time::second_clock::local_time() );
        boost::shared_ptr<std::string> message( new std::string( boost::posix_time::to_simple_string( pt ) ) );
        
        socket_.async_send_to( boost::asio::buffer(*message)
                               , remote_endpoint_
                               , boost::bind( &dgram_server::handle_send
                                              , this
                                              , message
                                              , boost::asio::placeholders::error
                                              , boost::asio::placeholders::bytes_transferred ) );
*/
        start_receive();
    }
}

void
dgram_server::handle_send( boost::shared_ptr<std::string>
                           , const boost::system::error_code&
                           , std::size_t )
{
}

void
dgram_server::handle_send_to( const boost::system::error_code& error )
{
    std::cout << "dgram_server::handle_send_to" << std::endl;
    if ( !error ) {
        timer_.expires_from_now( boost::posix_time::seconds( 3 ) );
        timer_.async_wait( boost::bind( &dgram_server::handle_timeout, this
                                        , boost::asio::placeholders::error ) );
    }
}

void
dgram_server::handle_timeout( const boost::system::error_code& error )
{
    if ( ! error ) {
        boost::array<char, 512> dbuf;
        new ( dbuf.data() ) LifeCycleFrame( DATA );
        boost::uint16_t * pseq = reinterpret_cast< boost::uint16_t * >( &dbuf[ sizeof( LifeCycleFrame ) ] );
        *pseq++ = ++local_seq_;
        
        size_t dlen = reinterpret_cast<char *>(pseq) - dbuf.data();

        std::cout << "dgram_server::handle_timeout send " << dlen << " bytes, seq=" << local_seq_ - 1 << std::endl;
        
        socket_.async_send_to( boost::asio::buffer( dbuf.data(), dlen )
                               , remote_endpoint_
                               , boost::bind( &dgram_server::handle_send_to, this
                                              , boost::asio::placeholders::error ) );
    }

}
