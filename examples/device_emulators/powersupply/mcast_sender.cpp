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

#include "mcast_sender.hpp"
#include <boost/bind.hpp>

const short multicast_port = 20001;
const int max_message_count = 999;

mcast_sender::mcast_sender( boost::asio::io_service& io_service
                            , const boost::asio::ip::address& mcast_address )
    : endpoint_( mcast_address, multicast_port )
    , socket_( io_service, endpoint_.protocol() )
    , timer_( io_service )
    , message_count_(0)
{
    std::ostringstream os;
    os << "HELLO " << message_count_++;
    message_ = os.str();

    socket_.async_send_to( boost::asio::buffer( message_ )
                           , endpoint_
                           , boost::bind( &mcast_sender::handle_send_to, this
                                          , boost::asio::placeholders::error ) );
}

void
mcast_sender::handle_send_to( const boost::system::error_code& error )
{
    std::cout << "mcast_sender::handle_send_to" << std::endl;
    if ( ! error && message_count_ < max_message_count ) {
        timer_.expires_from_now( boost::posix_time::seconds( 3 ) );
        timer_.async_wait( boost::bind( &mcast_sender::handle_timeout, this
                                        , boost::asio::placeholders::error ) );
    }
}

void
mcast_sender::handle_timeout( const boost::system::error_code& error )
{
    std::cout << "mcast_sender::handle_timeout " << message_count_ << std::endl;
    if ( ! error ) {
        std::ostringstream os;
        os << "HELLO " << message_count_++;
        message_ = os.str();
        socket_.async_send_to( boost::asio::buffer( message_ )
                               , endpoint_
                               , boost::bind( &mcast_sender::handle_send_to, this
                                              , boost::asio::placeholders::error ) );        
    }
}
