//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "adcontroller.h"
#pragma warning (disable: 4996)
#include "ace/Init_ACE.h"
#pragma warning (default: 4996)

#if defined ACE_WIN32
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
#     pragma comment(lib, "xmlwrapperd.lib")
#     pragma comment(lib, "adplugind.lib")
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
#     pragma comment(lib, "xmlwrapper.lib")
#     pragma comment(lib, "adplugin.lib")
#  endif
#endif

#pragma warning (disable: 4996)
# include <ace/SOCK_Dgram_Mcast.h>
# include <ace/Service_Config.h>
# include <ace/Sched_Params.h>
# include <ace/Thread_Manager.h>
# include <ace/Process_Manager.h>
# include <ace/OS.h>
# include <tao/Utils/ORB_Manager.h>
#pragma warning (default: 4996)

#include <acewrapper/nameservice.h>

#include "signal_handler.h"
#include <signal.h>
#include <acewrapper/orbservant.h>

#include <iostream>
#include <fstream>

#include <acewrapper/constants.h>
#include <boost/smart_ptr.hpp>
#include "manager_i.h"
#include "ibrokermanager.h"

using namespace acewrapper;

static int debug_flag = 1;
static bool __aborted = false;
static bool __own_thread = false;

static Receiver * __preceiver_debug;
std::string __ior_session;

//-----------------------------------------------

adController::~adController()
{
}

adController::adController()
{
}

adController::operator bool () const
{ 
	return true;
}

void
adController::_abort_server()
{
	__aborted = true;
	adController::_deactivate();
}

bool
adController::initialize( CORBA::ORB_ptr orb )
{
	if ( ! orb ) {
		int ac = 0;
		orb = CORBA::ORB_init( ac, 0 );
	}

	ORBServant< adcontroller::manager_i > * pServant = adcontroller::singleton::manager::instance();
	ORBServantManager * pMgr = new ORBServantManager( orb );
	pMgr->init( 0, 0 );
	pServant->setServantManager( pMgr );
	return true;
}

void
adController::initial_reference( const char * iorBroker )
{
    adcontroller::singleton::manager::instance()->broker_manager_reference( iorBroker );
}

const char *
adController::activate()
{
	ORBServant< adcontroller::manager_i > * pServant = adcontroller::singleton::manager::instance();
	pServant->activate();
    return pServant->ior().c_str();
}

bool
adController::deactivate()
{
	return adController::_deactivate();
}

bool
adController::_deactivate()
{
	ORBServant< adcontroller::manager_i > * pServant = adcontroller::singleton::manager::instance();
	pServant->deactivate();

	adcontroller::singleton::iBrokerManager::instance()->manager_terminate();

	return true;
}

int
adController::run()
{
   ACE_Sched_Params fifo_sched_params( ACE_SCHED_FIFO, 
				       ACE_Sched_Params::priority_min( ACE_SCHED_FIFO ),
				       ACE_SCOPE_PROCESS );
   
   //<---- set real time priority class for this process
   if ( debug_flag == 0 ) {
	   if ( ACE_OS::sched_params(fifo_sched_params) == -1 ) {
		   if ( errno == EPERM || errno == ENOTSUP ) 
			   ACE_DEBUG((LM_DEBUG, "Warning: user's not superuser, so we'll run in the theme-shared class\n"));
		   else
			   ACE_ERROR_RETURN((LM_ERROR, "%p\n", "ACE_OS::sched_params()"), -1);
	   }
   } else {
	   std::cerr << "==================================================" << std::endl;
	   std::cerr << "====== running normal priority for debug =========" << std::endl;
	   std::cerr << "==================================================" << std::endl;
   }
   //-------------> end priority code

   ORBServantManager* p = adcontroller::singleton::manager::instance()->getServantManager();
   if ( p->test_and_set_thread_flag() ) {
	   __own_thread = true;
	   ACE_Thread_Manager::instance()->spawn( ACE_THR_FUNC( ORBServantManager::thread_entry ), reinterpret_cast<void *>(p) );
   }

   return 0;
}

void
adController::abort_server()
{
	adController::_abort_server();
}

/////////////////////

__declspec(dllexport) bool initialize( CORBA::ORB * orb )
{
	return adController().initialize( orb );
}

__declspec(dllexport) bool activate()
{
	return adController().activate();
}

__declspec(dllexport) bool deactivate()
{
	return adController().deactivate();
}

__declspec(dllexport) int run()
{
	return adController().run();
}

__declspec(dllexport) void abort_server()
{
	return adController().abort_server();
}
