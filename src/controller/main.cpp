//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include <QtGui/QApplication>
#include "maincontrollerwindow.h"

#include "ace/Init_ACE.h"

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
#endif

#pragma warning (disable: 4996)
#include "session_i.h"
#include <acewrapper/orbservant.h>
#include <acewrapper/nameservice.h>
#include <ace/Thread_Manager.h>
#pragma warning (default: 4996)

typedef ACE_Singleton< acewrapper::ORBServant< session_i >, ACE_Recursive_Thread_Mutex > ORBImpl;

using namespace acewrapper;

void orb_shutdown()
{
	ORBImpl::instance()->deactivate();
    ORBImpl::instance()->getServantManager()->fini();
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainControllerWindow w;

    w.on_initial_update();
    w.show();

	try {
        CORBA::ORB_var orb = CORBA::ORB_init( argc, argv );
        acewrapper::ORBServantManager * pMgr = new acewrapper::ORBServantManager( orb );
        pMgr->init( argc, argv );

        ORBServant< session_i > * pServant = ORBImpl::instance();
        pServant->setServantManager( pMgr );

		pServant->activate();
		CosNaming::Name name;
        name.length(1);
		name[0].id = "controller.controller";
		CORBA::Object_var obj = *ORBImpl::instance();
		NS::register_name_service( orb, name, obj );
        pMgr->spawn();
	} catch ( CORBA::Exception& ) {
	}
    return a.exec();
}
