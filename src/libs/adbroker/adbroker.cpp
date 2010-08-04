//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "adbroker.h"
#pragma warning (disable: 4996)
# include "ace/Init_ACE.h"
#pragma warning (default: 4996)

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

#pragma warning (disable: 4996)
# include <tao/Utils/ORB_Manager.h>
# include <ace/Thread_Manager.h>
# include <ace/OS.h>
# include <ace/Process_Manager.h>
#pragma warning (default: 4996)

#include <acewrapper/nameservice.h>
#include <acewrapper/orbservant.h>
#include <acewrapper/constants.h>
#include <acewrapper/acewrapper.h>

#include "manager_i.h"

using namespace acewrapper;

static bool __own_thread;

adbroker::adbroker(void)
{
}

adbroker::~adbroker(void)
{
}

void
adbroker::abort_server()
{
	deactivate();
	if ( __own_thread )
		singleton::broker::manager::instance()->getServantManager()->fini();
}

bool
adbroker::initialize( CORBA::ORB_ptr orb )
{
	if ( ! orb ) {
		int ac = 0;
		orb = CORBA::ORB_init( ac, 0 );
	}
	ORBServant< manager_i > * pServant = singleton::broker::manager::instance();
	ORBServantManager * pMgr = new ORBServantManager( orb );
	pMgr->init( 0, 0 );
	pServant->setServantManager( pMgr );
	return true;
}

bool
adbroker::activate()
{
	ORBServant< manager_i > * pServant = singleton::broker::manager::instance();
	pServant->activate();

	CORBA::ORB_var orb = pServant->getServantManager()->orb();
	CosNaming::Name name = acewrapper::constants::adbroker::manager::name();
	return NS::register_name_service( orb.in(), name, *pServant );
}

bool
adbroker::deactivate()
{
	ORBServant< manager_i > * pServant = singleton::broker::manager::instance();
	pServant->deactivate();
	return true;
}

int
adbroker::run()
{
	ORBServantManager* p = singleton::broker::manager::instance()->getServantManager();
	if ( p->test_and_set_thread_flag() ) {
        __own_thread = true;
		ACE_Thread_Manager::instance()->spawn( ACE_THR_FUNC( ORBServantManager::thread_entry ), reinterpret_cast<void *>(p) );
	}
    return 0;
}

