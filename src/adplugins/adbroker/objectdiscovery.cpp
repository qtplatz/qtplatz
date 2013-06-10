// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "objectdiscovery.hpp"
#include <acewrapper/iorquery.hpp>

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
#include <boost/bind.hpp>
#include <thread>
#include <functional>

using namespace adbroker;

#define IORQ "ior?"

ObjectDiscovery::ObjectDiscovery() : reactor_thread_( 0 )
                                   , io_service_( new boost::asio::io_service )
                                   , suspend_( false )
{
	iorQuery_.reset( 
        new acewrapper::iorQuery( *io_service_
                                  , boost::bind( &ObjectDiscovery::reply_handler
                                                 , this, _1, _2 ) ) );
}

ObjectDiscovery::~ObjectDiscovery()
{
}

void
ObjectDiscovery::close()
{
    adportable::debug(__FILE__, __LINE__)
        << "============= ObjectDiscovery::close() ===============";
    if ( iorQuery_ )
        iorQuery_->close();
    if ( io_service_ )
        io_service_->stop();
    if ( thread_ )
        thread_->join();
}

bool
ObjectDiscovery::open()
{
    iorQuery_->open();
	std::lock_guard< std::mutex > lock( mutex_ );
    if ( ! thread_ ) {
        thread_.reset(
            new std::thread( 
                boost::bind( &boost::asio::io_service::run
                             , io_service_.get() ) )
            );
    }
	return true;
}

void
ObjectDiscovery::reply_handler( const std::string& ident, const std::string& ior )
{
	std::string name;
    if ( unregister_lookup( ident, name ) ) {
        adportable::debug() << "ObjectDiscovery: name=" << name << ", " << ident 
                            << " ior=" << ior.substr(0, 20) << "...";
        manager_i::instance()->internal_register_ior( name, ior );
	}
}

void
ObjectDiscovery::register_lookup( const std::string& name, const std::string& ident )
{
	std::lock_guard< std::mutex > lock( mutex_ );
    list_[ ident ] = name;
    iorQuery_->resume();
}

bool
ObjectDiscovery::unregister_lookup( const std::string& ident, std::string& name )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    std::map< std::string, std::string >::iterator it = list_.find( ident );
    if ( it != list_.end() ) {
        name = it->second;
        list_.erase( ident );
        if ( list_.empty() )
            iorQuery_->suspend();
        return true;
    }
    return false;
}

//////////
