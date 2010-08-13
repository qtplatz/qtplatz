//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "tofcontroller.h"
#include "i8tmanager.h"
#include <acewrapper/orbservant.h>  // servant template
#include <acewrapper/nameservice.h>
#include <acewrapper/constants.h>

static bool __aborted = false;
static bool __own_thread = false;

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

TofController::~TofController()
{
}

TofController::TofController()
{
}

bool
TofController::initialize( CORBA::ORB * orb )
{
	if ( ! orb ) {
		int ac = 0;
		orb = CORBA::ORB_init( ac, 0 );
	}

	acewrapper::ORBServant< tofcontroller::i8tManager_i >
		* pServant = tofcontroller::singleton::i8tManager_i::instance();
	acewrapper::ORBServantManager * pMgr = new acewrapper::ORBServantManager( orb );
	pMgr->init( 0, 0 );
	pServant->setServantManager( pMgr );

	return true;
}

bool
TofController::activate()
{
	acewrapper::ORBServant< tofcontroller::i8tManager_i > 
		* pServant = tofcontroller::singleton::i8tManager_i::instance();
	pServant->activate();

	CORBA::ORB_var orb = pServant->getServantManager()->orb();
	CosNaming::Name name = acewrapper::constants::tofcontroller::manager::name();
	return acewrapper::NS::register_name_service( orb, name, *pServant );
}

bool
TofController::deactivate()
{
	acewrapper::ORBServant< tofcontroller::i8tManager_i >
		* pServant = tofcontroller::singleton::i8tManager_i::instance();
	pServant->deactivate();

	// adcontroller::singleton::iBrokerManager::instance()->terminate();

	return true;
}

int
TofController::run()
{
    return 0;
}

void
TofController::abort_server()
{
	__aborted = true;
	deactivate();
}

adplugin::orbLoader * instance()
{
	return new TofController();
}

__declspec(dllexport) bool initialize( CORBA::ORB * orb )
{
	return TofController().initialize( orb );
}

__declspec(dllexport) bool activate()
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
