//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

#include "tofcontroller.h"
#include "tofsession_i.h"
#include <acewrapper/orbservant.h>  // servant template
#include <acewrapper/constants.h>

static bool __aborted = false;
static bool __own_thread = false;

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
#     pragma comment(lib, "adplugind.lib")
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
#     pragma comment(lib, "adplugin.lib")
#  endif

TofController::~TofController()
{
}

TofController::TofController()
{
}

TofController::operator bool () const
{ 
	return true;
}

bool
TofController::initialize( CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, PortableServer::POAManager_ptr mgr)
{
	acewrapper::ORBServant< tofcontroller::tofSession_i >
		* pServant = tofcontroller::singleton::tofSession_i::instance();
    pServant->initialize( orb, poa, mgr );
/*
	acewrapper::ORBServantManager * pMgr = new acewrapper::ORBServantManager( orb );
	pMgr->init( 0, 0 );
	pServant->setServantManager( pMgr );
*/
	return true;
}

void
TofController::initial_reference( const char * ior_broker_manager )
{
    using namespace tofcontroller;
	singleton::tofSession_i::instance()->broker_manager_ior( ior_broker_manager );
}

const char *
TofController::activate()
{
	acewrapper::ORBServant< tofcontroller::tofSession_i > 
		* pServant = tofcontroller::singleton::tofSession_i::instance();
	pServant->activate();
	return pServant->ior().c_str();
}

bool
TofController::deactivate()
{
	tofcontroller::singleton::tofSession_i::instance()->deactivate();
	return true;
}

adplugin::orbLoader * instance()
{
	return new TofController();
}

/*
__declspec(dllexport) bool initialize( CORBA::ORB * orb )
{
	return TofController().initialize( orb );
}

__declspec(dllexport) void initial_reference( const char * iorBroker )
{
	TofController().initial_reference( iorBroker );
}

__declspec(dllexport) const char * activate()
{
	return TofController().activate();
}

__declspec(dllexport) bool deactivate()
{
	return TofController().deactivate();
}

__declspec(dllexport) int run()
{
	return TofController().run();
}

__declspec(dllexport) void abort_server()
{
	return TofController().abort_server();
}
*/