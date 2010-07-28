#include "nameservice.h"
#include <tao/ORB.h>
#include <orbsvcs/CosNamingC.h>

using namespace acewrapper;

CosNaming::NamingContext_ptr
NS::resolve_init( CORBA::ORB_ptr orb )
{
	CORBA::Object_var obj;
	try {
		obj = orb->resolve_initial_references( "NameService" );
	} catch ( const CORBA::ORB::InvalidName& ) {
		throw;
	} catch ( const CORBA::Exception& ex ) {
		ex._tao_print_exception( "NS::resolve_initial_references\t\n" );
	}

	CosNaming::NamingContext::_var_type ref;
	try {
		ref = CosNaming::NamingContext::_narrow( obj.in() );
	} catch ( const CORBA::Exception& ex ) {
		ex._tao_print_exception( "NS::resolve_initial_references\t\n" );
	}
	return ref._retn();
}

bool
NS::bind(CosNaming::NamingContext_ptr nc, const CosNaming::Name &name, CORBA::Object_ptr obj)
{
	if ( CORBA::is_nil( nc ) )
		return false;
	nc->bind( name, obj );
	return true;
}

CORBA::Object_ptr
NS::resolve_name(CosNaming::NamingContext_ptr nc, const CosNaming::Name &name )
{
	CORBA::Object_var obj;
	try {
		obj = nc->resolve( name );
	} catch ( const CosNaming::NamingContext::NotFound& ) {
		throw;
	}
	return obj._retn();
}

// static
bool
NS::register_name_service( CORBA::ORB_ptr orb, const CosNaming::Name& name, CORBA::Object_ptr obj )
{
	if ( CORBA::is_nil( orb ) )
		return false;

	CosNaming::NamingContext_var nc;
	try { 
		nc = NS::resolve_init( orb );
	} catch ( const CORBA::Exception& ex ) {
		ex._tao_print_exception( "register_name_service" );
        return false;
	}

	try {
		nc->bind( name, obj );
	} catch ( const CosNaming::NamingContext::AlreadyBound& ex ) {
		ex._tao_print_exception( "register_name_service" );
		return true; // ignore
	} catch ( const CORBA::Exception& ex ) {
		ex._tao_print_exception( "register_name_service" );
        return false;
	}
	return true;
}