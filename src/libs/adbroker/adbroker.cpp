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
#     pragma comment(lib, "TAOd.lib")
#     pragma comment(lib, "ACEd.lib")
#     pragma comment(lib, "adinterfaced.lib")
#     pragma comment(lib, "adportabled.lib")
#     pragma comment(lib, "acewrapperd.lib")
#     pragma comment(lib, "adcontrolsd.lib")
#     pragma comment(lib, "portfoliod.lib")
#  else
#     pragma comment(lib, "TAO_Utils.lib")
#     pragma comment(lib, "TAO_PI.lib")
#     pragma comment(lib, "TAO_PortableServer.lib")
#     pragma comment(lib, "TAO_AnyTypeCode.lib")
#     pragma comment(lib, "TAO.lib")
#     pragma comment(lib, "ACE.lib")
#     pragma comment(lib, "adinterface.lib")
#     pragma comment(lib, "adportable.lib")
#     pragma comment(lib, "acewrapper.lib")
#     pragma comment(lib, "adcontrols.lib")
#     pragma comment(lib, "portfolio.lib")
#  endif

#pragma warning (disable: 4996)
# include <tao/Utils/ORB_Manager.h>
# include <ace/Thread_Manager.h>
# include <ace/Process_Manager.h>
#pragma warning (default: 4996)

#include <acewrapper/orbservant.h>
#include <acewrapper/constants.h>
#include <acewrapper/acewrapper.h>
#include <adportable/debug.hpp>

#include "manager_i.h"
#include "brokermanager.h"

using namespace acewrapper;

static bool __own_thread;

adBroker::adBroker(void)
{
}

adBroker::~adBroker(void)
{
}

void
adBroker::abort_server()
{
	deactivate();
}

bool
adBroker::initialize( CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, PortableServer::POAManager_ptr mgr )
{
    ORBServant< adbroker::manager_i > * pServant = adbroker::singleton::manager::instance();
	pServant->initialize( orb, poa, mgr );
	return true;
}

const char *
adBroker::activate()
{
    ORBServant< adbroker::manager_i > * pServant = adbroker::singleton::manager::instance();
	pServant->activate();
    return pServant->ior().c_str();
}

bool
adBroker::deactivate()
{
	adbroker::BrokerManager::terminate();
	adbroker::singleton::manager::instance()->deactivate();
	return true;
}

int
adBroker::run()
{
/*
    ORBServantManager* p = adbroker::singleton::manager::instance()->getServantManager();
	if ( p->test_and_set_thread_flag() ) {
        __own_thread = true;
		ACE_Thread_Manager::instance()->spawn( ACE_THR_FUNC( ORBServantManager::thread_entry ), reinterpret_cast<void *>(p) );
        ACE_OS::sleep(0);
	}
*/
    return 0;
}

