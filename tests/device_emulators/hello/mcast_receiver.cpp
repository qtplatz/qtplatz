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

#include "mcast_receiver.hpp"
#include "lifecycle.hpp"
#include <boost/bind.hpp>

const short multicast_port = 20001;

mcast_receiver::mcast_receiver( boost::asio::io_service& io_service
                                , lifecycle& lifecycle
                                , const boost::asio::ip::address& listen_address
                                , const boost::asio::ip::address& mcast_address )
    : socket_(io_service) 
    , lifecycle_( lifecycle )
{
    boost::asio::ip::udp::endpoint listen_endpoint( listen_address, multicast_port );
    socket_.open( listen_endpoint.protocol() );
    socket_.set_option( boost::asio::ip::udp::socket::reuse_address( true ) );
    socket_.bind( listen_endpoint );

    // join to mcast
    socket_.set_option( boost::asio::ip::multicast::join_group( mcast_address ) );

    socket_.async_receive_from( boost::asio::buffer( data_, max_length )
                                , sender_endpoint_
                                , boost::bind( &mcast_receiver::handle_receive_from, this
                                               , boost::asio::placeholders::error
                                               , boost::asio::placeholders::bytes_transferred ) );
}

void
mcast_receiver::handle_receive_from( const boost::system::error_code& error, size_t bytes_recvd )
{
    if ( ! error ) {
        lifecycle_( sender_endpoint_, data_, bytes_recvd );
        socket_.async_receive_from( boost::asio::buffer( data_, max_length )
                                    , sender_endpoint_
                                    , boost::bind( &mcast_receiver::handle_receive_from, this
                                                   , boost::asio::placeholders::error
                                                   , boost::asio::placeholders::bytes_transferred ) );
    }
}
