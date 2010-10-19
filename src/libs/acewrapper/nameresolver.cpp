//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "nameresolver.h"
#pragma warning (disable: 4996)
# include <tao/Object.h>
# include <orbsvcs/CosNamingC.h>
#pragma warning (default: 4996)

using namespace acewrapper;

nameresolver::nameresolver(void)
{
}

nameresolver::~nameresolver(void)
{
}

CORBA::Object_ptr
nameresolver::resolve_name( CORBA::ORB_ptr orb, const std::string& ns_name )
{
	CORBA::Object_var obj;
/*
	CosNaming::NamingContext_var nc = resolve_init( orb );
	if ( ! CORBA::is_nil( nc.in() ) ) {

		CosNaming::Name name;
        name.length(1);
		name[0].id = CORBA::string_dup( adportable::string::convert( ns_name ).c_str() );
		name[0].kind = CORBA::string_dup("");

		try {
			obj = nc->resolve( name );
		} catch ( const CosNaming::NamingContext::NotFound& ) {
			throw;
		}
	}
*/
	return obj._retn();
}



// static
bool
nameresolver::register_name_service( const std::string& name, const std::string& ior )
{
/*
	CosNaming::NamingContext_var nc;
	try { 
		nc = NS::resolve_init( orb );
	} catch ( const CORBA::Exception& ex ) {
		ex._tao_print_exception( "register_name_service" );
        return false;
	}

    if ( CORBA::is_nil( nc ) )
        return false;

	try {
		nc->rebind( name, obj );
	} catch ( const CosNaming::NamingContext::AlreadyBound& ex ) {
		ex._tao_print_exception( "register_name_service" );
	} catch ( const CORBA::Exception& ex ) {
		ex._tao_print_exception( "register_name_service" );
        return false;
	}
*/
	return true;
}

// static
bool
nameresolver::unregister_name_service( const std::string& name )
{
/*
	CosNaming::NamingContext_var nc;
	try { 
		nc = NS::resolve_init( orb );
	} catch ( const CORBA::Exception& ex ) {
		ex._tao_print_exception( "register_name_service" );
        return false;
	}

	try {
		nc->unbind( name );
	} catch ( const CORBA::Exception& ex ) {
		ex._tao_print_exception( "register_name_service" );
        return false;
	}
*/
	return true;
}
