/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#include "iorquery.hpp"
#include <acewrapper/constants.hpp>
#include <acewrapper/ifconfig.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>

using namespace acewrapper;
using boost::asio::ip::udp;

static const u_short dstport = acewrapper::constants::adbroker::OBJECTDISCOVERY_PORT;

iorQuery::iorQuery( boost::asio::io_service& io_service
                    , boost::function<void (const std::string&, const std::string&)> handler )
    : io_service_( io_service )
    , handle_reply_( handler )
    , socket_( io_service_, udp::endpoint( udp::v4(), 0 ) )
    , timer_( io_service_ )
    , interval_( 3000 )
{
    socket_.set_option( boost::asio::socket_base::broadcast( true ) );
}

void
iorQuery::close()
{
    timer_.cancel();
}

void
iorQuery::initiate_timer()
{
    timer_.expires_from_now( boost::posix_time::milliseconds( interval_ ) );
    timer_.async_wait(
        boost::bind( &iorQuery::handle_timeout
                     , this
                     , boost::asio::placeholders::error )
        );
}

bool
iorQuery::open()
{
    timer_.cancel();
    initiate_timer();
    start_receive();
    send_query();
    return true;
}

void
iorQuery::handle_timeout( const boost::system::error_code& error )
{
    if ( ! error ) {
        send_query();
        initiate_timer();
    }
}

void
iorQuery::handle_receive( const boost::system::error_code& error, std::size_t len )
{
    if ( ! error ) {
        if ( len >= 4 ) {
            std::string reply( recv_buffer_.data() );
            std::string::size_type crlf = reply.find_first_of( "\r\n" );
            if ( crlf != std::string::npos ) {
                std::string ident = reply.substr( 0, crlf );
                std::string ior = reply.substr( crlf + 1 );
                handle_reply_( ident, ior );
            }
        }
        start_receive();
    }
}

void
iorQuery::send_query()
{
    acewrapper::ifconfig::ifvec vec;
    if ( acewrapper::ifconfig::broadaddr( vec ) ) {
        BOOST_FOREACH( acewrapper::ifconfig::ifaddr& addr, vec ) {
            boost::asio::ip::udp::endpoint dest( boost::asio::ip::address::from_string( addr.second ), dstport );
            socket_.send_to( boost::asio::buffer("ior?"), dest );
        }
    } else {
        boost::asio::ip::udp::endpoint dest( boost::asio::ip::address_v4::any(), dstport );        
        socket_.send_to( boost::asio::buffer("ior?"), dest );
    }
}

void
iorQuery::start_receive()
{
    socket_.async_receive_from( boost::asio::buffer( recv_buffer_ )
                                , endpoint_
                                , boost::bind( &iorQuery::handle_receive
                                               , this
                                               , boost::asio::placeholders::error
                                               , boost::asio::placeholders::bytes_transferred ) );
}
