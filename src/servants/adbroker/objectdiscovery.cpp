// This is a -*- C++ -*- header.
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

#include "objectdiscovery.hpp"
#include "manager_i.hpp"
#include <acewrapper/constants.hpp>
#include <acewrapper/reactorthread.hpp>
#include <acewrapper/mutex.hpp>
#include <acewrapper/ifconfig.hpp>
#include <adportable/debug.hpp>
#include <iostream>

#include <ace/Thread.h>
#include <ace/Log_Msg.h>
#include <ace/Event_Handler.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Dgram_Bcast.h>
#include <boost/foreach.hpp>

using namespace adbroker;

#define IORQ "ior?"

namespace adbroker {

	class TimerHandler : public ACE_Event_Handler {
	public:
		TimerHandler( ObjectDiscovery& t ) : parent_( t ) {
		}
		virtual int handle_timeout( const ACE_Time_Value&, const void * ) {
			return parent_.handle_timeout();
		}
	private:
		ObjectDiscovery& parent_;
	};

	class BcastHandler : public ACE_Event_Handler {
    public:
        BcastHandler( ObjectDiscovery& );
        virtual int handle_input( ACE_HANDLE );
        virtual int handle_close( ACE_HANDLE, ACE_Reactor_Mask );
        virtual ACE_HANDLE get_handle() const;
        bool open();
        bool close();
        bool send( const char *, ssize_t );
        bool send( const char *, ssize_t, const ACE_INET_Addr& );
        bool recv( char *, ssize_t bufsize, ACE_INET_Addr& );
		inline acewrapper::ifconfig::ifvec& ifvec() { return ifvec_; }
    private:
		acewrapper::ifconfig::ifvec ifvec_;
        ACE_SOCK_Dgram_Bcast sock_bcast_;
		ACE_INET_Addr sock_addr_;
        ObjectDiscovery& parent_;
    };

} // namespace adbroker

ObjectDiscovery::ObjectDiscovery( ACE_Recursive_Thread_Mutex& mutex )
: reactor_thread_( 0 )
, timer_( new TimerHandler( *this ) )
, bcast_( new BcastHandler( *this ) )
, suspend_( false )
, mutex_( mutex )
{
}

ObjectDiscovery::~ObjectDiscovery()
{
    delete reactor_thread_;
    delete bcast_;
	delete timer_;
}

void
ObjectDiscovery::close()
{
    adportable::debug(__FILE__, __LINE__) << "============= ObjectDiscovery::close() ===============";

    if ( reactor_thread_ == 0 )
        return;

    acewrapper::scoped_mutex_t<> lock( mutex_ );
    if ( reactor_thread_ ) {
        bcast_->close();
        reactor_thread_->get_reactor()->cancel_timer( bcast_ );
        reactor_thread_->end_reactor_event_loop();
        bool res = reactor_thread_->join();
        adportable::debug(__FILE__, __LINE__) 
            << "===== ObjectDiscovery::close(): thread->join() call " << ( res ? "success" : "fail" );
    }
}

bool
ObjectDiscovery::open()
{
    if ( reactor_thread_ )
        return false;

	acewrapper::ifconfig::broadaddr( bcast_->ifvec() );

    acewrapper::scoped_mutex_t<> lock( mutex_ );
    if ( reactor_thread_ == 0 ) {
        reactor_thread_ = new acewrapper::ReactorThread();
		bcast_->open();
        ACE_Reactor * reactor = reactor_thread_->get_reactor();
        reactor->register_handler( bcast_, ACE_Event_Handler::READ_MASK );
		reactor->schedule_timer( timer_, 0, ACE_Time_Value(1), ACE_Time_Value(3) );
        return reactor_thread_->spawn();
    }
    return false;
}

void
ObjectDiscovery::operator()( const char * pbuf, int, const ACE_INET_Addr& from )
{
#if defined DEBUG //&& 0
    std::cout << "***** ObjectDiscovery:  from " << from.get_host_addr() << ":" << from.get_port_number() << std::endl;
#endif
    ACE_UNUSED_ARG( from );
    std::string reply( pbuf );
    std::string::size_type pos = reply.find_first_of( "\r\n" );
    
    if ( pos != std::string::npos ) {

        std::string ident = reply.substr( 0, pos );
        std::string ior = reply.substr( pos + 1 );
        std::string name;
        if ( unregister_lookup( ident, name ) ) {
            adportable::debug() << "ObjectDiscovery: name=" << name << ", " << ident 
                                << " ior=" << ior.substr(0, 20) << "...";
            manager_i::instance()->internal_register_ior( name, ior );
        }
    }
}

int
ObjectDiscovery::handle_timeout( )
{
	if ( ! suspend_ )
        bcast_->send( IORQ, sizeof( IORQ ) );
    return 0;
}

void
ObjectDiscovery::register_lookup( const std::string& name, const std::string& ident )
{
    acewrapper::scoped_mutex_t<> lock( mutex_ );
    list_[ ident ] = name;
    suspend_ = false;
    handle_timeout(); 
}

bool
ObjectDiscovery::unregister_lookup( const std::string& ident, std::string& name )
{
    acewrapper::scoped_mutex_t<> lock( mutex_ );
    std::map< std::string, std::string >::iterator it = list_.find( ident );
    if ( it != list_.end() ) {
        name = it->second;
        list_.erase( ident );
        suspend_ = list_.empty();
        return true;
    }
    return false;
}

//////////

BcastHandler::BcastHandler( ObjectDiscovery& t ) : sock_addr_( u_short(0) )
                                                 , parent_( t )
{
}

ACE_HANDLE
BcastHandler::get_handle() const
{
	return sock_bcast_.get_handle();
}

int
BcastHandler::handle_input( ACE_HANDLE )
{
    char buf[2048];
    memset( buf, 0, sizeof(buf) );

    ACE_INET_Addr remote_addr;

    ssize_t res = sock_bcast_.recv( buf, sizeof(buf), remote_addr );

    adportable::debug() << "BcastHandler::handle_input from " 
                        << remote_addr.get_host_addr() << ":" << remote_addr.get_port_number()
                        << "\n\"" << std::string(buf).substr(0, 60) << "...\"";

    if ( res >= 4 && ( strncmp(buf, IORQ, sizeof(IORQ) - 1) != 0 ) ) {
        parent_( buf, res, remote_addr );
        return 0;
    }
    return -1; // error
}

int
BcastHandler::handle_close( ACE_HANDLE, ACE_Reactor_Mask )
{
    return 0;
}

bool
BcastHandler::open()
{
    int res = sock_bcast_.open( sock_addr_ );

    adportable::debug() << "----- BcastHandler::open " 
                        << sock_addr_.get_host_addr() << ":" << sock_addr_.get_port_number()
                        << " res= " << res << " -----";

    return res != (-1);
}

bool
BcastHandler::close()
{
    return sock_bcast_.close();
}

bool
BcastHandler::send( const char * pbuf, ssize_t size )
{
    const unsigned int sport = acewrapper::constants::adbroker::OBJECTDISCOVERY_PORT;

	if ( ifvec_.empty() ) {
		static ACE_INET_Addr to( sport, INADDR_BROADCAST );
		ssize_t ret = send( pbuf, size , to );
		return ret == size;
	}
	BOOST_FOREACH( acewrapper::ifconfig::ifaddr& ifaddr, ifvec_ ) {
		ACE_INET_Addr to( sport, ifaddr.second.c_str() );
		send( pbuf, size, to );
	}
	return true;
}

bool
BcastHandler::send( const char * pbuf, ssize_t size, const ACE_INET_Addr& to )
{
    ssize_t ret = sock_bcast_.send( pbuf, size , const_cast<ACE_INET_Addr&>(to) );

#if defined DEBUG && 0
    static int count;
    adportable::debug() << "[" << count++ << "]BcastHandler::send(" << pbuf << ", " << to.get_host_addr() << ":" 
                        << int(to.get_port_number()) << ") ret=" << int(ret);
#endif
    return ret == size;
}

bool
BcastHandler::recv( char * pbuf, ssize_t bufsize, ACE_INET_Addr& remote_addr )
{
    return sock_bcast_.recv( pbuf, bufsize, remote_addr );
}
