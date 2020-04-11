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
#include <adportable/debug.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>

using namespace acewrapper;
using boost::asio::ip::udp;

udpEventSender::udpEventSender( boost::asio::io_service& io
                                , const char * host
                                , const char * port
                                , bool bcast ) : sock_( io, udp::endpoint( udp::v4(), 0 ) )
{
    if ( bcast ) {
        sock_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        sock_.set_option(boost::asio::socket_base::broadcast(true));
    }

    udp::resolver resolver( io );
    endpoint_ = *resolver.resolve( { udp::v4(), host, port } );

    std::ostringstream o;
    o << endpoint_;

    ADDEBUG() << "udpEventSender endpoint: " << o.str() << "\tbroadcast: " << bcast;
}

bool
udpEventSender::send_to( const std::string& data, std::function< void( result_code, double, const char *) > callback )
{

    ADDEBUG() << "send_to: " << data;
    auto tp = std::chrono::system_clock::now();
    try {
        sock_.send_to( boost::asio::buffer( data.c_str(), data.size() ), endpoint_ );
    } catch ( const boost::system::system_error &ex ) {
        ADDEBUG() << ex.what();
        return false;
    }

    // block ack wait
    std::size_t transferred = 0;
    boost::system::error_code ec = boost::asio::error::would_block;

    std::unique_lock< std::mutex > lock( mutex_ );
#if BOOST_VERSION >= 107000
    boost::asio::steady_timer timer( sock_.get_executor() );
#else
    boost::asio::steady_timer timer( sock_.get_io_service() );
#endif
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

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - tp).count();
    double elapsed_time = double(duration) / std::micro::den; // to seconds

    if ( transferred == 0 ) { // timed out
        callback( transaction_timeout, elapsed_time, ec.message().c_str() );
        return false;
    }

    callback( transaction_completed, elapsed_time, "success" );

    return true;
}
