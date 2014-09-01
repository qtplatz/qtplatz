#include <QtWidgets/QApplication>
#include "mainwindow.h"
#include "tofsession_i.h"
#include <acewrapper/acewrapper.h>
#include <tao/ORB.h>
#include <tao/Utils/ORB_Manager.h>
#include <ace/Thread_Manager.h>
#include <ace/OS_NS_unistd.h>

//////////////////
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
#  endif

void *
orb_thread_entry( void * me )
{
    TAO_ORB_Manager * orbmgr = static_cast< TAO_ORB_Manager * >(me);
    orbmgr->run();
    return 0;
}


int
main(int argc, char *argv[])
{
    acewrapper::instance_manager::initialize();

    QApplication a(argc, argv);

    //----------------------------------------//
    TAO_ORB_Manager orbmgr;
    orbmgr.init( argc, argv );
    acewrapper::ORBServant< tofcontroller::tofSession_i > * pServant = tofcontroller::singleton::tofSession_i::instance();

    ACE_Thread_Manager::instance()->spawn( ACE_THR_FUNC( orb_thread_entry ), reinterpret_cast<void *>(&orbmgr) );
    ACE_OS::sleep(1);

    pServant->initialize( orbmgr.orb(), orbmgr.root_poa(), orbmgr.poa_manager() );
    pServant->activate();
    std::string ior = pServant->ior();
    //----------------------------------------//
    Instrument::Session_var session;
    int trial = 50;
    do {
        try {
            CORBA::Object_var obj = orbmgr.orb()->string_to_object( ior.c_str() );
            session = Instrument::Session::_narrow( obj.in() );
            break;
        } catch ( CORBA::Exception& ) {
            ACE_OS::sleep(0);
        }
    } while( trial-- );

    MainWindow w;

    w.setSession( session.in() );

    w.show();
    a.exec();

    pServant->deactivate();
    return 0;
}
