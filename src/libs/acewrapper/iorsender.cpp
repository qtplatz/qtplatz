/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#include "iorsender.hpp"
#include <acewrapper/constants.hpp>
#include <acewrapper/ifconfig.hpp>
#include <acewrapper/mutex.hpp>
#include <adportable/debug.hpp>
#include <boost/bind.hpp>
#include <ace/Thread.h>
#include <ace/Thread_Manager.h>

using namespace acewrapper;

namespace acewrapper { namespace singleton {
	typedef ACE_Singleton< iorSender, ACE_Recursive_Thread_Mutex > iorSender;
    }
}

static const u_short srcport = 8111;
static const u_short dstport = acewrapper::constants::adbroker::OBJECTDISCOVERY_PORT;

using boost::asio::ip::udp;

iorSender::iorSender() : thread_running_( false )
		       , t_handle_( 0 )
		       , socket_( io_service_, udp::endpoint( udp::v4(), dstport ) )
		       , nextIor_( iorvec_.end() )
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
    if ( t_handle_ ) {
	io_service_.stop();
	ACE_Thread::join( t_handle_, 0, 0 );
	t_handle_ = 0;
    }
}

void
iorSender::register_lookup( const std::string& ior, const std::string& ident )
{
    iorvec_[ ident ] = ior;
#ifdef _DEBUG
    std::cout << "## register_lookup: " << ident << " size=" << iorvec_.size() << std::endl;
#endif
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
#if defined _DEBUG
    std::cout << "## iorSender start_receive from: "
	      << sender_endpoint_.address().to_string() 
	      << ":" << sender_endpoint_.port() << std::endl;
#endif
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
#if defined _DEBUG || defined DEBUG
        std::cout << "********** iorSender::handle_timeout: " << error << std::endl;
#endif
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

    adportable::debug( __FILE__, __LINE__ ) << "## iorSender::handle_receive " << query << " ##";

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

        adportable::debug( __FILE__, __LINE__ ) << "## iorSender::handle_sendto("
            << sender_endpoint_.address().to_string() << "." << sender_endpoint_.port()
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
    if ( ! thread_running_ ) {
	acewrapper::scoped_mutex_t<> lock( mutex_ );
	if ( ! thread_running_ ) {
	    thread_running_ = true;
	    ::ACE_Thread_Manager::instance()->spawn( ACE_THR_FUNC( iorSender::thread_entry ), this );
	    return true;
	}
    }
    return false;
}

// static
void *
iorSender::thread_entry( void * )
{
    iorSender * pThis = instance();
    pThis->t_handle_ = ACE_Thread::self();

    pThis->io_service_.run();

    return 0;
}

// static
iorSender *
iorSender::instance()
{
    return singleton::iorSender::instance();
}
