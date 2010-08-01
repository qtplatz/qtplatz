//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "adbroker.h"
#include "ace/Init_ACE.h"

#  if defined _DEBUG
#     pragma comment(lib, "TAO_Utilsd.lib")
#     pragma comment(lib, "TAO_PId.lib")
#     pragma comment(lib, "TAO_PortableServerd.lib")
#     pragma comment(lib, "TAO_AnyTypeCoded.lib")
#     pragma comment(lib, "TAO_CosNamingd.lib")
#     pragma comment(lib, "TAOd.lib")
#     pragma comment(lib, "ACEd.lib")
#     pragma comment(lib, "adinterfaced.lib")
#     pragma comment(lib, "adportabled.lib")
#     pragma comment(lib, "acewrapperd.lib")
#  else
#     pragma comment(lib, "TAO_Utils.lib")
#     pragma comment(lib, "TAO_PI.lib")
#     pragma comment(lib, "TAO_PortableServer.lib")
#     pragma comment(lib, "TAO_AnyTypeCode.lib")
#     pragma comment(lib, "TAO_CosNaming.lib")
#     pragma comment(lib, "TAO.lib")
#     pragma comment(lib, "ACE.lib")
#     pragma comment(lib, "adinterface.lib")
#     pragma comment(lib, "adportable.lib")
#     pragma comment(lib, "acewrapper.lib")
#  endif

#include <tao/Utils/ORB_Manager.h>
#include <ace/Thread_Manager.h>
#include <ace/OS.h>
#include <ace/Process_Manager.h>

#include <acewrapper/nameservice.h>
#include <acewrapper/orbservant.h>
#include <acewrapper/constants.h>
#include <acewrapper/acewrapper.h>

#include "manager_i.h"

using namespace acewrapper;

adbroker::adbroker(void)
{
}

adbroker::~adbroker(void)
{
}

void
adbroker::abort_server()
{
    static_cast<manager_i *>( *singleton::manager::instance() )->shutdown();
    singleton::manager::instance()->deactivate();
    singleton::manager::instance()->fini();
}

bool
register_name_service( const CosNaming::Name& name )
{
    CORBA::ORB_var orb = singleton::manager::instance()->orb();

	if ( CORBA::is_nil( orb.in() ) )
		return false;

    return NS::register_name_service( orb, name, *singleton::manager::instance() );
}

int
adbroker::run( int argc, ACE_TCHAR * argv[] )
{
    acewrapper::instance_manager::initialize();
    try {
        int ret;
        // -ORBListenEndpoints iiop://192.168.0.1:9999
        ret = singleton::manager::instance()->init(argc, argv);
        singleton::manager::instance()->activate();

        if ( ret != 0 )
            ACE_ERROR_RETURN( (LM_ERROR, "\n error in init.\n"), 1 );

        register_name_service( acewrapper::constants::adbroker::manager::name() );
      
    } catch ( const CORBA::Exception& ex ) {
        ex._tao_print_exception( "run\t\n" );
        // return 1;
    }

    ORBServant< manager_i > * p = singleton::manager::instance();
    ACE_Thread_Manager::instance()->spawn( ACE_THR_FUNC( ORBServant<manager_i>::thread_entry ), reinterpret_cast<void *>(p) );

    return 0;
}

CORBA::ORB_ptr
adbroker::orb()
{
    return singleton::manager::instance()->orb();
}

const char *
adbroker::ior()
{
    return singleton::manager::instance()->ior().c_str();
}
