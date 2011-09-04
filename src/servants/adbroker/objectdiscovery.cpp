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
#include <acewrapper/mutex.hpp>
#include <adportable/debug.hpp>
#include <ace/Thread.h>
#include <ace/Log_Msg.h>
#include <ace/Event_Handler.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Dgram_Mcast.h>
#include <ace/SOCK_Dgram_Bcast.h>
#include <iostream>

using namespace adbroker;

#define IORQ "ior?"

namespace adbroker {

// class McastHandler : public ACE_Event_Handler {
// public:
//     McastHandler( ObjectDiscovery& );
//     virtual int handle_input( ACE_HANDLE );
//     virtual int handle_close( ACE_HANDLE, ACE_Reactor_Mask );
//     virtual ACE_HANDLE get_handle() const;
//     virtual int handle_timeout( const ACE_Time_Value&, const void * arg );
//     bool open( u_short );
//     bool close();
//     bool send( const char *, ssize_t );
//     bool recv( char *, ssize_t bufsize, ACE_INET_Addr& );

// private:
//     ACE_SOCK_Dgram_Mcast sock_mcast_;
//     ACE_INET_Addr sock_addr_;
//     ObjectDiscovery& parent_;
// };

    class BcastHandler : public ACE_Event_Handler {
    public:
        BcastHandler( ObjectDiscovery& );
        virtual int handle_input( ACE_HANDLE );
        virtual int handle_close( ACE_HANDLE, ACE_Reactor_Mask );
        virtual ACE_HANDLE get_handle() const;
        virtual int handle_timeout( const ACE_Time_Value&, const void * arg );
        bool open();
        bool close();
        bool send( const char *, ssize_t );
        bool send( const char *, ssize_t, const ACE_INET_Addr& );
        bool recv( char *, ssize_t bufsize, ACE_INET_Addr& );
    private:
        // ACE_SOCK_Dgram sock_dgram_;
        ACE_SOCK_Dgram_Bcast sock_bcast_;
        ACE_INET_Addr sock_addr_;
        ObjectDiscovery& parent_;
    };

} // namespace adbroker

ObjectDiscovery::ObjectDiscovery( ACE_Recursive_Thread_Mutex& mutex ) : t_handle_( 0 )
                                                                      , reactor_( new ACE_Reactor )
                                                                        // , mcast_( new McastHandler( *this) )
                                                                      , bcast_( new BcastHandler( *this) )
                                                                      , suspend_( false )
                                                                      , mutex_( mutex )
{
}

ObjectDiscovery::~ObjectDiscovery()
{
    delete bcast_;
    // delete mcast_;
#ifndef WIN32 // workaround to avoid clash on close app.
    delete reactor_;
#endif
}

void *
ObjectDiscovery::thread_entry( void * me )
{
    reinterpret_cast< ObjectDiscovery *>(me)->event_loop();
    return 0;
}

void
ObjectDiscovery::event_loop()
{
    t_handle_ = ACE_Thread::self();
    reactor_->owner( t_handle_ );

    // reactor_->register_handler( mcast_, ACE_Event_Handler::READ_MASK );
    reactor_->register_handler( bcast_, ACE_Event_Handler::READ_MASK );
    reactor_->schedule_timer( bcast_, 0, ACE_Time_Value(1), ACE_Time_Value(3) );

    while ( reactor_->handle_events() >= 0 ) 
        ;

    reactor_->cancel_timer( bcast_ );
    reactor_->close();

    adportable::debug(__FILE__, __LINE__) << "===== ObjectDiscovery::event_loop done =====";
}

void
ObjectDiscovery::close()
{
    adportable::debug() << "============= ObjectDiscovery::close() ===============";

    // mcast_->close();
    bcast_->close();

    if ( t_handle_ ) {
        reactor_->end_reactor_event_loop();
        int res = ACE_Thread::join( t_handle_, 0, 0 );

        adportable::debug(__FILE__, __LINE__) 
            << "===== ObjectDiscovery::close join " << ( res == 0 ? "success" : "failed" );
    }
}

bool
ObjectDiscovery::open()
{
    return bcast_->open();
    // return mcast_->open( port );
}

void
ObjectDiscovery::operator()( const char * pbuf, ssize_t, const ACE_INET_Addr& from )
{
#if defined DEBUG && 0
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
ObjectDiscovery::handle_timeout()
{
#if defined DEBUG && 0
    std::cerr << "****** adborker::ObjectDiscovery:handle_timeout() ******** " 
              << (suspend_ ? " suspended" : "broadcasting" ) << std::endl;
#endif
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

    // force broadcast
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

// McastHandler::McastHandler( ObjectDiscovery& t) : sock_addr_( ACE_DEFAULT_MULTICAST_PORT, ACE_DEFAULT_MULTICAST_ADDR )
//                                                 , parent_( t )
// {
// }

// ACE_HANDLE
// McastHandler::get_handle() const
// {
//     return sock_mcast_.get_handle();
// }

// int
// McastHandler::handle_input( ACE_HANDLE )
// {
//     char buf[1024];
//     memset( buf, 0, sizeof( buf ) );

//     adportable::debug() << "McastHandler::handle_input";

//     ACE_INET_Addr remote_addr;
//     ssize_t res = sock_mcast_.recv( buf, sizeof(buf), remote_addr );
//     if ( res != (-1) ) {
//         parent_( buf, res, remote_addr );
//         return 0;
//     }
//     return -1; // error
// }

// int
// McastHandler::handle_timeout( const ACE_Time_Value&, const void * )
// {
//     if ( ! parent_.suspend() )
//         return parent_.handle_timeout();
//     return 0;
// }

// int
// McastHandler::handle_close( ACE_HANDLE, ACE_Reactor_Mask )
// {
//     return 0;
// }

// bool
// McastHandler::open( u_short port )
// {
//     if ( port )
//         sock_addr_.set_port_number( port );
//     if ( sock_mcast_.join( sock_addr_ ) == (-1) ) {
//         adportable::debug(__FILE__, __LINE__) << "McastHandler::open: can't subscribe to multicast group";
//         return false;
//     }
//     return true;
// }

// bool
// McastHandler::close()
// {
//     return sock_mcast_.close();
// }

// bool
// McastHandler::send( const char * pbuf, ssize_t size )
// {
//     ssize_t ret = sock_mcast_.send( pbuf, size );
//     adportable::debug() << "McastHandler::send(" << pbuf << ", " << size << ") ret=" << ret;
//     return ret == size;
// }

// bool
// McastHandler::recv( char * pbuf, ssize_t bufsize, ACE_INET_Addr& remote_addr )
// {
//     return sock_mcast_.recv( pbuf, bufsize, remote_addr );
// }

///////////////////////////
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
BcastHandler::handle_timeout( const ACE_Time_Value&, const void * )
{
    return parent_.handle_timeout();
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
    static ACE_INET_Addr to( 7402, INADDR_BROADCAST );
    ssize_t ret = send( pbuf, size , to );
    return ret == size;
}

bool
BcastHandler::send( const char * pbuf, ssize_t size, const ACE_INET_Addr& to )
{
    ssize_t ret = sock_bcast_.send( pbuf, size , const_cast<ACE_INET_Addr&>(to) );

    static int count;
#if defined DEBUG
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
