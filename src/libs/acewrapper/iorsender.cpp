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

#include "iorsender.hpp"
#include <acewrapper/constants.hpp>
#include <acewrapper/ifconfig.hpp>
#include <acewrapper/mutex.hpp>
#include <adportable/debug.hpp>
#include <boost/bind.hpp>
#include <ace/Thread.h>
#include <ace/Thread_Manager.h>

using namespace acewrapper;

enum { debug_ior_sender = 0 };

namespace acewrapper { namespace singleton {
	typedef ACE_Singleton< iorSender, ACE_Recursive_Thread_Mutex > iorSender;
    }
}

static const u_short srcport = 8111;
static const u_short dstport = acewrapper::constants::adbroker::OBJECTDISCOVERY_PORT;

using boost::asio::ip::udp;

iorSender::iorSender() : socket_( io_service_, udp::endpoint( udp::v4(), dstport ) )
		       , nextIor_( iorvec_.end() )
                       , thread_( 0 )
{
    send_buffer_.reserve( 1500 );
    socket_.set_option( boost::asio::socket_base::broadcast( true ) );

    acewrapper::ifconfig::ifvec vec;
    if ( acewrapper::ifconfig::broadaddr( vec ) ) {
	acewrapper::ifconfig::ifaddr& addr = vec[0];

	sender_endpoint_ = udp::endpoint( boost::asio::ip::address::from_string( addr.second ), srcport );
    } else {
	sender_endpoint_ = udp::endpoint( boost::asio::ip::address_v4::any(), srcport );
    }

    start_receive();
}

void
iorSender::close()
{
    if ( thread_ ) {
        boost::mutex::scoped_lock lock( mutex_ );
        if ( thread_ ) {
            io_service_.stop();
            thread_->join();
            delete thread_;
            thread_ = 0;
        }
    }
}

void
iorSender::register_lookup( const std::string& ior, const std::string& ident )
{
    iorvec_[ ident ] = ior;
}

void
iorSender::unregister_lookup( const std::string& ident )
{
    std::map< std::string, std::string>::iterator it = iorvec_.find( ident );
    if ( it != iorvec_.end() ) 
        iorvec_.erase( it );
}

void
iorSender::start_receive()
{
    socket_.async_receive_from( boost::asio::buffer( recv_buffer_ )
				, sender_endpoint_
				, boost::bind( &iorSender::handle_receive
					       , this
					       , boost::asio::placeholders::error
					       , boost::asio::placeholders::bytes_transferred ) );

    //recv_timer_.expires_from_now( boost::posix_time::seconds( 3 ) );
    //recv_timer_.async_wait( boost::bind( &iorSender::handle_timeout, this, boost::asio::placeholders::error ) );
}

void
iorSender::handle_timeout( const boost::system::error_code& error )
{
    if ( ! error ) {
#if defined DEBUG && 0
	recv_timer_.expires_from_now( boost::posix_time::seconds( 3 ) );
	recv_timer_.async_wait( boost::bind( &iorSender::handle_timeout, this, boost::asio::placeholders::error ) );

	send_buffer_.resize(8);
	socket_.async_send_to( boost::asio::buffer( send_buffer_ )
			       , sender_endpoint_
			       , boost::bind( &iorSender::handle_sendto
					      , this
					      , boost::asio::placeholders::error ) );
#endif
    } else {
        adportable::debug( __FILE__, __LINE__ ) << "ERROR *** iorSender::handle_timeout: " << error;
    }

}

void
iorSender::handle_receive( const boost::system::error_code& error, std::size_t len )
{
    if ( ! error || error == boost::asio::error::message_size ) {

        const char * query = recv_buffer_.data();

        if ( debug_ior_sender )
            adportable::debug( __FILE__, __LINE__ ) << "## handle_receive (" << query << ") ##";

        if ( std::strncmp( query, "ior?", len ) == 0 ) {

            if ( iorvec_.empty() )
                return;

            if ( nextIor_ != iorvec_.end() ) // still remain
                return;

            nextIor_ = iorvec_.begin();
            handle_sendto( boost::system::error_code() ); // force sendto
        } 
        start_receive();
    }
}

void
iorSender::handle_sendto( const boost::system::error_code& error )
{
    if ( ! error ) {
        if ( nextIor_ != iorvec_.end() ) {
            std::string reply( nextIor_->first + "\n" + nextIor_->second );
            send_buffer_.resize( reply.size() + 1 );
            std::strcpy( &send_buffer_[0], reply.c_str() );

            if ( debug_ior_sender )
                adportable::debug( __FILE__, __LINE__ ) << "## handle_sendto("
                                                        << sender_endpoint_.address().to_string()
                                                        << "." << sender_endpoint_.port()
                                                        << ") ##\n" << &send_buffer_[0];

            socket_.async_send_to( boost::asio::buffer( send_buffer_ )
                                   , sender_endpoint_
                                   , boost::bind( &iorSender::handle_sendto
                                                  , this
                                                  , boost::asio::placeholders::error ) );	    

            ++nextIor_;
        }
    }
}

bool
iorSender::spawn()
{
    if ( thread_ == 0 ) {
        boost::mutex::scoped_lock lock( mutex_ );
	if ( thread_ == 0 ) {
            thread_ = new boost::thread( boost::bind( & boost::asio::io_service::run, &io_service_ ) );
	    return true;
	}
    }
    return false;
}

// static
iorSender *
iorSender::instance()
{
    return singleton::iorSender::instance();
}
