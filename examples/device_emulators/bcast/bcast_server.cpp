/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

#include "bcast_server.hpp"
#include "lifecycle.hpp"
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using boost::asio::ip::udp;

bcast_server::bcast_server( boost::asio::io_service& io_service, int port )
    : socket_( io_service, udp::endpoint( udp::v4(), port ) )
    , timer_( io_service )
    , local_seq_( 0x100 )
    , remote_seq_( 0 )
{
    boost::asio::socket_base::broadcast option(true);
    socket_.set_option( option );
    send_buffer_.reserve( 1500 );
    start_receive();
}

void
bcast_server::start_receive()
{
    socket_.async_receive_from( boost::asio::buffer( recv_buffer_ )
                                , remote_endpoint_
                                , boost::bind( &bcast_server::handle_receive
                                               , this
                                               , boost::asio::placeholders::error
                                               , boost::asio::placeholders::bytes_transferred ) );
}

void
bcast_server::handle_receive( const boost::system::error_code& error, std::size_t len )
{
    using adportable::protocol::CONN_SYN;
    using adportable::protocol::CONN_SYN_ACK;
    using adportable::protocol::DATA;
    using adportable::protocol::DATA_ACK;

    if ( ! error || error == boost::asio::error::message_size ) {

        const size_t fsize = sizeof( LifeCycleFrame );

        const LifeCycleFrame * pf = reinterpret_cast< const LifeCycleFrame *>( recv_buffer_.data() );
        if ( len >= fsize + 2 ) {
            const boost::uint16_t * pseq = reinterpret_cast< const boost::uint16_t *>( &recv_buffer_[ fsize ] );

            std::cout << "bcast/bcast_server::handle_receive command: " << std::hex << pf->command
                      << " " << len 
                      << " bytes from: " << remote_endpoint_.address().to_string() 
                      << "/" << std::dec << remote_endpoint_.port()
                      << std::endl;

            if ( pf->command == CONN_SYN ) {
                std::cout << "CONN_SYN:" << " lseq: " << pseq[0] << " rseq: " << pseq[1] << std::endl;
                boost::array< char, sizeof( LifeCycleFrame ) + 4 > dbuf;
                new ( dbuf.data() ) LifeCycleFrame( CONN_SYN_ACK );
                boost::uint16_t * pack = reinterpret_cast< boost::uint16_t * >( &dbuf[ sizeof( LifeCycleFrame ) ] );
                *pack++ = local_seq_ = 0x201;
                *pack++ = remote_seq_ = pseq[0];
                async_send_data( dbuf.data(), dbuf.size() );

            } else if ( pf->command == CONN_SYN_ACK ) {
                std::cout << "CONN_SYN|ACK received " << " lseq: " << pseq[0] << " rseq: " << pseq[1] << std::endl;
                // timer_.expires_from_now( boost::posix_time::seconds( 3 ) );
                // timer_.async_wait( boost::bind( &bcast_server::handle_timeout, this
                //                                 , boost::asio::placeholders::error ) );
            } else if ( pf->command == DATA_ACK ) {
                std::cout << "DATA|ACK received " << " lseq: " << pseq[0] << " rseq: " << pseq[1] << std::endl;
            } else if ( pf->command == DATA ) {
                std::cout << "DATA received " << " lseq: " << pseq[0] << " flags: " << pseq[1] << std::endl;
                const boost::uint32_t * data = reinterpret_cast< boost::uint32_t *>( &recv_buffer_[ fsize ] );
                const char * pcmd = reinterpret_cast< const char *>(&data[2]);
                std::string xcmd;
                for ( int i = 0; i < 4; ++i )
                    xcmd += pcmd[i];
                std::cout << "\t" << xcmd << ": " << std::hex << std::showbase;
                for ( size_t i = 0; i < (len - fsize) / sizeof(boost::uint32_t); ++i )
                    std::cout << data[i] << " ";
                std::cout << std::endl;
            } else {
                std::cout << "UNKNOWN received " << std::hex 
                          << pf->command << " lseq: " << pseq[0] << " rseq: " << pseq[1] << std::endl;
                std::cout << " CONN_SYN = " << std::hex << CONN_SYN << std::endl;
            }
        }

        start_receive();
    }
}

void
bcast_server::handle_send( boost::shared_ptr<std::string>
                           , const boost::system::error_code&
                           , std::size_t )
{
}

void
bcast_server::handle_send_to( const boost::system::error_code& error )
{
    std::cout << "bcast_server::handle_send_to" << std::endl;
    if ( !error ) {
        timer_.expires_from_now( boost::posix_time::seconds( 3 ) );
        timer_.async_wait( boost::bind( &bcast_server::handle_timeout, this
                                        , boost::asio::placeholders::error ) );
    }
}

void
bcast_server::handle_timeout( const boost::system::error_code& error )
{
    if ( ! error ) {
        std::cout << "bcast/bcast_server::handle_timeout" << std::endl;
/*
        boost::array<char, 512> dbuf;
        new ( dbuf.data() ) LifeCycleFrame( adportable::protocol::DATA );
        boost::uint16_t * pseq = reinterpret_cast< boost::uint16_t * >( &dbuf[ sizeof( LifeCycleFrame ) ] );
        *pseq++ = ++local_seq_;
        
        size_t dlen = reinterpret_cast<char *>(pseq) - dbuf.data();

        std::cout << "bcast_server::handle_timeout send " << dlen << " bytes, seq=" << local_seq_ - 1 << std::endl;
        
        socket_.async_send_to( boost::asio::buffer( dbuf.data(), dlen )
                               , remote_endpoint_
                               , boost::bind( &bcast_server::handle_send_to, this
                                              , boost::asio::placeholders::error ) );
*/
    }

}

bool
bcast_server::async_send_data( const char * pbuf, std::size_t size )
{
    send_buffer_.resize( size );
    memcpy( send_buffer_.data(), pbuf, size );
    try {
        socket_.async_send_to( boost::asio::buffer( send_buffer_ ), remote_endpoint_
                               , boost::bind( &bcast_server::handle_send_to, this
                                              , boost::asio::placeholders::error ) );        
    } catch ( std::exception& ex ) {
        std::cout << "bcast/bcast_server::async_send_data exception: " << ex.what() << std::endl;
    }
    return true;
}
