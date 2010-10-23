//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "brokerhelper.h"
#pragma warning (disable: 4996)
# include <tao/Object.h>
# include <adinterface/brokerC.h>
#pragma warning (default: 4996)

using namespace acewrapper;

brokerhelper::brokerhelper()
{
}

brokerhelper::~brokerhelper(void)
{
}

//static
Broker::Manager_ptr
brokerhelper::getManager( CORBA::ORB_ptr orb, const std::string& iorBrokerMgr )
{
    CORBA::Object_var obj = orb->string_to_object( iorBrokerMgr.c_str() );
    return Broker::Manager::_narrow( obj );
}

//static
CORBA::Object * 
brokerhelper::name_to_object( CORBA::ORB_ptr orb, const std::string& name, const std::string& iorBroker  )
{
    Broker::Manager_var mgr = getManager( orb, iorBroker );
    if ( ! CORBA::is_nil( mgr ) ) {
        CORBA::Object_var obj = orb->string_to_object( mgr->ior( name.c_str() ) );
        return obj._retn();
    }
    return 0;
}

//static
std::string
brokerhelper::ior( Broker::Manager * mgr, const char * name )
{
    if ( mgr ) {
        CORBA::String_var str = mgr->ior( name );
        return std::string( str );
    }
    return "";
}


