// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "manager_i.hpp"
#include "session_i.hpp"
#include "logger_i.hpp"
#include "objectdiscovery.hpp"
#include <adportable/debug.hpp>
#include <adportable/string.hpp>
#include <acewrapper/mutex.hpp>
#include <ace/Thread_Manager.h>

using namespace adbroker;

adbroker::manager_i::manager_i(void) : discovery_(0)
{
    adportable::debug() << "adbroker::manager_i ctor";
}

adbroker::manager_i::~manager_i(void)
{
    adportable::debug() << "adbroker::~manager_i dtor";
    delete discovery_;
}

void
adbroker::manager_i::shutdown()
{
    adportable::debug() << "####################################################";
    adportable::debug() << "##### adbroker::manager::shutting down ... ######";

    if ( discovery_ ) {
        acewrapper::scoped_mutex_t<> lock( mutex_ );
        if ( discovery_ )
            discovery_->close();
    }

    PortableServer::POA_var poa = adbroker::singleton::manager::instance()->poa();
    if ( logger_i_ )
        poa->deactivate_object( logger_i_->oid() );

    adportable::debug() << "##### adbroker::manager::shutdown complete ######";
    adportable::debug() << "####################################################";
}

Broker::Session_ptr
manager_i::getSession( const CORBA::WChar * token )
{
    PortableServer::POA_var poa = ::adbroker::singleton::manager::instance()->poa();

    if ( CORBA::is_nil( poa ) )
        return 0;

    session_map_type::iterator it = session_list_.find( token );
    if ( it == session_list_.end() )
        session_list_[ token ].reset( new adbroker::session_i( token ) );

    CORBA::Object_ptr obj = poa->servant_to_reference( session_list_[ token ].get() );
    try {
        return Broker::Session::_narrow( obj ); // return new object, refcount should be 1
    } catch ( CORBA::Exception& ) {
        return 0;
    }
}

Broker::Logger_ptr
manager_i::getLogger()
{
    PortableServer::POA_var poa = ::adbroker::singleton::manager::instance()->poa();
    if ( CORBA::is_nil( poa ) )
        return 0;

    if ( ! logger_i_ ) {
        logger_i_.reset( new broker::logger_i( poa ) );
        PortableServer::ObjectId * oid = poa->activate_object( logger_i_.get() );
        logger_i_->oid( *oid );
    }

    CORBA::Object_var obj = poa->servant_to_reference( logger_i_.get() );
    try {
        return Broker::Logger::_narrow( obj );
    } catch ( CORBA::Exception& ) {
        return 0;
    }
}

void
manager_i::register_ior( const char * name, const char * ior )
{
    adportable::debug() << "adbroker::manager_i::register_ior(" << std::string(name) << ", " << std::string(ior) << ")";

    acewrapper::scoped_mutex_t<> lock( mutex_ );
    iorMap_[ name ] = ior;
}

char *
manager_i::ior( const char * name )
{
    acewrapper::scoped_mutex_t<> lock( mutex_ );

    std::map< std::string, std::string >::iterator it = iorMap_.find( name );
    if ( it != iorMap_.end() )
        return CORBA::string_dup( it->second.c_str() );
    return 0;
}

void
manager_i::register_lookup( const char * name, const char * ident )
{
    if ( discovery_ == 0 ) {
        acewrapper::scoped_mutex_t<> lock( mutex_ );
        if ( discovery_ == 0 ) {
            discovery_ = new ObjectDiscovery( mutex_ );
            ACE_Thread_Manager::instance()->spawn( ACE_THR_FUNC( ObjectDiscovery::thread_entry ), discovery_ );
        }
    }
    if ( discovery_ ) {
        acewrapper::scoped_mutex_t<> lock( mutex_ );
        discovery_->registor_lookup( name, ident );    
        lookup_[ name ] = ident;
    } while(0);

    adportable::debug() << "================================================================";
    adportable::debug() << "======== adbroker::manager_i::register_lookup(" 
                        << std::string(name) << ", " << std::string(ident) << ")";
}
