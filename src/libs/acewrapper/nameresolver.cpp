//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "nameresolver.h"
#pragma warning (disable: 4996)
# include <tao/Object.h>
// # include <orbsvcs/CosNamingC.h>
# include <adinterface/brokerC.h>
#pragma warning (default: 4996)

using namespace acewrapper;

nameresolver::nameresolver(void)
{
}

nameresolver::~nameresolver(void)
{
}

//static
Broker::Manager *
nameresolver::getManager( CORBA::ORB_ptr orb, const std::string& ior )
{
	CORBA::Object_var obj = orb->string_to_object( ior.c_str() );
	return Broker::Manager::_narrow( obj );
}

//static
std::string
nameresolver::ior( Broker::Manager * mgr, const char * name )
{
	if ( mgr ) {
		CORBA::String_var str = mgr->ior( name );
		return std::string( str );
	}
	return "";
}

