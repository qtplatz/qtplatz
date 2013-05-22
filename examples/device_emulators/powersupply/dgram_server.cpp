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
#include "../hello/lifecycle.hpp"
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using boost::asio::ip::udp;

dgram_server::dgram_server( boost::asio::io_service& io_service
                            , boost::asio::ip::udp::endpoint& remote
                            , unsigned short rseq )
    : socket_( io_service, udp::endpoint( udp::v4(), 7100 ) )
    , remote_endpoint_( remote )
    , remote_seq_( rseq )
    , local_seq_( 0x200 )
{
    start_receive();
}

void
dgram_server::conn_syn()
{
    // reply CONN|SYN|ACK
    boost::array< char, sizeof( LifeCycleFrame ) + 4 > frame;
    new( frame.data() ) LifeCycleFrame( CONN_SYN_ACK );
    boost::uint16_t * pseq = reinterpret_cast< boost::uint16_t *>( &frame[ sizeof(LifeCycleFrame) ] );
    *pseq++ = local_seq_++;
    *pseq++ = remote_seq_;

    std::cout << "dgram_server: CONN_SYN_ACK reply to " 
              << remote_endpoint_.address() << "/" 
              << remote_endpoint_.port() << std::endl;
    socket_.send_to( boost::asio::buffer( frame ), remote_endpoint_ );
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
dgram_server::handle_receive( const boost::system::error_code& error, std::size_t len )
{
    const size_t fsize = sizeof( LifeCycleFrame );

    if ( ! error || error == boost::asio::error::message_size ) {
        
        std::cout << "dgram receive from: " 
                  << remote_endpoint_.address().to_string() 
                  << "/" << remote_endpoint_.port()
                  << std::endl;

        if ( len >= sizeof( LifeCycleFrame ) ) {
            const LifeCycleFrame * pf = reinterpret_cast< const LifeCycleFrame *>( recv_buffer_.data() );

            if ( pf->command == DATA && len >= fsize + 2 ) {
                boost::uint16_t rseq = *reinterpret_cast< const boost::uint16_t *>( &recv_buffer_[ fsize ] );
                if ( rseq != remote_seq_ + 1 ) {
                    std::cout << "remote sequence got " << rseq << " while waiting " << remote_seq_ + 1 << std::endl;
                } else {
                    std::cout << "remote data " << rseq << " received successfully" << std::endl;
                    const size_t flen = fsize + 4;
                    
                    boost::shared_array< char > dbuf( new char [ flen ] );
                    new ( dbuf.get() ) LifeCycleFrame( DATA_ACK );
                    boost::uint16_t * pseq = reinterpret_cast< boost::uint16_t *>( dbuf.get() + fsize );
                    *pseq++ = local_seq_++;
                    *pseq++ = rseq;
                    socket_.async_send_to( boost::asio::buffer( dbuf.get(), flen )
                                           , remote_endpoint_
                                           , boost::bind( &dgram_server::handle_send
                                                          , this
                                                          , dbuf
                                                          , boost::asio::placeholders::error
                                                          , boost::asio::placeholders::bytes_transferred ) );
                }
                remote_seq_ = rseq;
            } else if ( pf->command == CLOSE && len >= fsize + 2 ) {
                // todo
            }
        }
        start_receive();
    }
}

void
dgram_server::handle_send( boost::shared_array<char>, const boost::system::error_code&, std::size_t )
{
}
