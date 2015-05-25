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

#include "udpeventsender.hpp"
#include <boost/asio/steady_timer.hpp>
#include <workaround/boost/asio.hpp>
#include <mutex>
#include <string>
#include <chrono>
#include <iostream>

using namespace acewrapper;
using boost::asio::ip::udp;

udpEventSender::udpEventSender( boost::asio::io_service& io, const char * host, const char * port ) : sock_( io, udp::endpoint( udp::v4(), 0 ) )
{
    udp::resolver resolver( io );
    endpoint_ = *resolver.resolve( { udp::v4(), host, port } );
    std::cerr << endpoint_ << std::endl;
} 

bool
udpEventSender::send_to( const std::string& data, std::function< void( result_code, double, const char *) > callback )
{
    auto tp = std::chrono::steady_clock::now();

    sock_.send_to( boost::asio::buffer( data.c_str(), data.size() ), endpoint_ );

    // block ack wait
    std::size_t transferred = 0;
    boost::system::error_code ec = boost::asio::error::would_block;

    std::unique_lock< std::mutex > lock( mutex_ );

    boost::asio::steady_timer timer( sock_.get_io_service() );
    timer.expires_from_now( std::chrono::milliseconds( 500 ) ); // 0.5s

    timer.async_wait( [&]( const boost::system::error_code& ec ){
            if ( !ec ) { // timeout
                sock_.cancel();
                std::lock_guard< std::mutex > lock( mutex_ );
                cv_.notify_one();
            }
        } );

    char reply[ max_length ];

    sock_.async_receive( boost::asio::buffer( reply, max_length )
                         , [&](const boost::system::error_code& _ec, size_t _length ){
                             timer.cancel();
                             ec = _ec;
                             transferred = _length;
                             std::lock_guard< std::mutex > lock( mutex_ ); // block until cv_.wait() below
                             cv_.notify_one();
                         });

    cv_.wait( lock );

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - tp).count();
    double elapsed_time = duration * 1.0e-6; // to seconds

    if ( transferred == 0 ) { // timed out
        callback( transaction_timeout, elapsed_time, ec.message().c_str() );
        return false;
    }

    callback( transaction_completed, elapsed_time, "success" );

    return true;
}

