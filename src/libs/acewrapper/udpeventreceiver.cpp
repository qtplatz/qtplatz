/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#pragma once

#include "udpeventreceiver.hpp"
#include <workaround/boost/asio.hpp>
#include <mutex>
#include <thread>
#include <vector>
#include <string>

using namespace acewrapper;
using boost::asio::ip::udp;

udpEventReceiver::udpEventReceiver( boost::asio::io_service& io, short port ) : io_service_( io )
                                                                              , sock_( io, udp::endpoint( udp::v4(), port ) )
{
    do_receive();
} 

void
udpEventReceiver::register_handler( std::function<void( const char *, size_t )> h )
{
    handler_ = h;
}

void
udpEventReceiver::do_receive()
{
    sock_.async_receive_from(
        boost::asio::buffer(data_, max_length), sender_endpoint_,
        [this](boost::system::error_code ec, std::size_t bytes_recvd)  {
            if (!ec && bytes_recvd > 0) {
                if ( handler_ )
                    handler_( data_, bytes_recvd );
                do_send(bytes_recvd); // echo back
            } else {
                do_receive();
            }
        });
}

void
udpEventReceiver::do_send(std::size_t length)
{
    sock_.async_send_to(
        boost::asio::buffer(data_, length), sender_endpoint_,
        [this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/) {
            do_receive();
        });
}
