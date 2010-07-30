//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "adcontroller.h"
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

#include <ace/SOCK_Dgram_Mcast.h>
#include <ace/Service_Config.h>
#include <ace/Sched_Params.h>
#include <ace/Thread_Manager.h>

#include <tao/Utils/ORB_Manager.h>
#include <acewrapper/nameservice.h>

#include "signal_handler.h"
#include <signal.h>
#include <acewrapper/orbservant.h>

#include <iostream>
#include <fstream>
#include <ace/OS.h>
#include <ace/Process_Manager.h>
#include <acewrapper/eventhandler.h>
#include <acewrapper/mcasthandler.h>
#include "mcast_handler.h"
#include <boost/smart_ptr.hpp>
#include <acewrapper/reactorthread.h>
#include <ace/Reactor.h>
#include "session_i.h"

using namespace acewrapper;

static int debug_flag = 1;
static bool __aborted = false;

void
adcontroller::abort_server()
{
	__aborted = true;
	singleton::session::instance()->deactivate();
}

bool
register_name_service()
{
	CORBA::ORB_var orb = singleton::session::instance()->orb();

	if ( CORBA::is_nil( orb.in() ) )
		return false;

	CosNaming::NamingContext_var nc;
	try { 
		nc = NS::resolve_init( orb );
	} catch ( const CORBA::Exception& ex ) {
		ex._tao_print_exception( "register_name_service" );
        return false;
	}

    // create a name
	CosNaming::Name name;
    name.length(1);
	name[0].id = CORBA::string_dup( "adics.session" );
	name[0].kind = CORBA::string_dup( "" );
 
	return NS::register_name_service( orb, name, *singleton::session::instance() );
}

int
adcontroller::run( int argc, ACE_TCHAR * argv[] )
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

   ReactorThread::spawn( TheReactorThread::instance() );
   
   boost::scoped_ptr< EventHandler< McastReceiver< mcast_handler > > > pmcast( new EventHandler< McastReceiver< mcast_handler > >() );
   if ( pmcast && pmcast->open() ) {
	   ACE_Reactor * reactor = TheReactorThread::instance()->get_reactor();
	   if ( reactor )
		   reactor->register_handler( pmcast.get(), ACE_Event_Handler::READ_MASK );
   }

   try {
      int ret;
      // -ORBListenEndpoints iiop://192.168.0.1:9999
	  ret = singleton::session::instance()->init(argc, argv);
	  singleton::session::instance()->activate();

      if ( ret != 0 )
          ACE_ERROR_RETURN( (LM_ERROR, "\n error in init.\n"), 1 );

      register_name_service();
      
   } catch ( const CORBA::Exception& ex ) {
       ex._tao_print_exception( "run\t\n" );
	   // return 1;
   }

   ORBServant< session_i > * p = singleton::session::instance();
   ACE_Thread_Manager::instance()->spawn( ACE_THR_FUNC( ORBServant<session_i>::thread_entry ), reinterpret_cast<void *>(p) );

   if ( ! __aborted )
	   singleton::session::instance()->deactivate();

   ACE_Reactor * reactor = TheReactorThread::instance()->get_reactor();
   reactor->end_reactor_event_loop();

   ACE_Thread_Manager::instance()->wait();

   reactor->close();
   return 0;
}

adcontroller::adcontroller()
{
}
