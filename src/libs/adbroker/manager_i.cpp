//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "manager_i.h"
#include "session_i.h"
#include "logger_i.h"

using namespace broker;

manager_i::manager_i(void) 
{
}

manager_i::~manager_i(void)
{
}

void
manager_i::shutdown()
{
    PortableServer::POA_ptr poa = singleton::manager::instance()->root_poa();

    if ( logger_i_ ) {
        poa->deactivate_object( logger_i_->oid() );
    }
}

Broker::Session_ptr
manager_i::getSession( const CORBA::WChar * token )
{
    PortableServer::POA_ptr poa = singleton::manager::instance()->root_poa();
    if ( CORBA::is_nil( poa ) )
        return 0;

    session_map_type::iterator it = session_list_.find( token );
    if ( it == session_list_.end() ) 
        session_list_[ token ].reset( new broker::session_i() );

    CORBA::Object_ptr obj = poa->servant_to_reference( session_list_[ token ].get() );
    return Broker::Session::_narrow( obj );
}

Broker::Logger_ptr
manager_i::getLogger()
{
    PortableServer::POA_ptr poa = singleton::manager::instance()->root_poa();
    if ( CORBA::is_nil( poa ) )
        return 0;

    if ( ! logger_i_ ) {
        logger_i_.reset( new broker::logger_i( poa ) );
        PortableServer::ObjectId * oid = poa->activate_object( logger_i_.get() );
        logger_i_->oid( *oid );
    }

    CORBA::Object_ptr obj = poa->servant_to_reference( logger_i_.get() );
    return Broker::Logger::_narrow( obj );
}