//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "manager_i.h"
#include "session_i.h"
#include "ibrokermanager.h"

using namespace adcontroller;

manager_i::manager_i(void) 
{
}

manager_i::~manager_i(void)
{
}

void
manager_i::shutdown()
{
	PortableServer::POA_var poa = singleton::manager::instance()->getServantManager()->root_poa();
}

ControlServer::Session_ptr
manager_i::getSession( const CORBA::WChar * token )
{
	PortableServer::POA_var poa = singleton::manager::instance()->getServantManager()->root_poa();

    if ( CORBA::is_nil( poa ) )
        return 0;

    if ( session_list_.empty() )
		adcontroller::singleton::iBrokerManager::instance()->manager_initialize();

    session_map_type::iterator it = session_list_.find( token );
    if ( it == session_list_.end() ) 
		session_list_[ token ].reset( new adcontroller::session_i() );

    CORBA::Object_ptr obj = poa->servant_to_reference( session_list_[ token ].get() );
	return ControlServer::Session::_narrow( obj );
}

