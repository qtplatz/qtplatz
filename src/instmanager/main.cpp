//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "ace/Init_ACE.h"

#if defined ACE_WIN32
#  if defined _DEBUG
#     pragma comment(lib, "TAO_Utilsd.lib")
#     pragma comment(lib, "TAO_PId.lib")
#     pragma comment(lib, "TAO_PortableServerd.lib")
#     pragma comment(lib, "TAO_AnyTypeCoded.lib")
#     pragma comment(lib, "TAOd.lib")
#     pragma comment(lib, "ACEd.lib")
// #     pragma comment(lib, "interfaced.lib")
#  else
#     pragma comment(lib, "TAO_Utils.lib")
#     pragma comment(lib, "TAO_PI.lib")
#     pragma comment(lib, "TAO_PortableServer.lib")
#     pragma comment(lib, "TAO_AnyTypeCode.lib")
#     pragma comment(lib, "TAO.lib")
#     pragma comment(lib, "ACE.lib")
// #     pragma comment(lib, "interface.lib")
#  endif
#endif

#include <ace/SOCK_Dgram_Mcast.h>
#include <ace/Service_Config.h>
#include <ace/Sched_Params.h>
#include <ace/Thread_Manager.h>

#include <tao/Utils/ORB_Manager.h>

#include "signal_handler.h"
#include <signal.h>
#include "orbserver.h"
#include <iostream>
#include <fstream>
#include "session_i.h"
#include <ace/OS.h>
#include <ace/Process_Manager.h>

static int debug_flag = 1;

int
run( int argc, ACE_TCHAR * argv[] )
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
   
   ACE_Thread_Manager * thrMgr = ACE_Thread_Manager::instance();
   
#if 0
   boost::scoped_ptr< ACE_Reactor > ucast_reactor( new ACE_Reactor() );
   boost::scoped_ptr< ACE_Reactor > mcast_reactor( new ACE_Reactor() );
   
   CCallbackImpl<CMcastServer> mcast( mcast_reactor.get() );
   CCallbackImpl<CDgramServer> dgram( ucast_reactor.get(), 7402 );
   
   // MULTI-CAST server start
   int tid0 = thrMgr->spawn( ACE_THR_FUNC( CMcastServer::thread_entry ),
			     reinterpret_cast<void *>(&mcast.server_) );
   
   // Unicast server start
   int tid1 = thrMgr->spawn( ACE_THR_FUNC( CDgramServer::thread_entry ),
			     reinterpret_cast<void *>(&dgram.server_) );
#endif
   
   orbserver<session_i> server;
   try {
      int ret;

      // -ORBListenEndpoints iiop://192.168.0.1:9999
      ret = server.init(argc, argv);

      if ( ret != 0 )
          ACE_ERROR_RETURN( (LM_ERROR, "\n error in init.\n"), 1 );
      
      std::string ior = server.activate();
      
      //mcast.ior_ = dgram.ior_ = ior;
      //mcast.server_.send( ior.c_str(), ior.size(), ACE_INET_Addr() );
      do {
          std::ofstream outf( "/var/tmp/imanager.ior" );
          outf << ior.c_str() << std::endl;
      } while (0);
   } catch ( const CORBA::Exception& ex ) {
       ex._tao_print_exception( "run\t\n" );
       return 1;
   }
   //int tidSession = thrMgr->spawn( ACE_THR_FUNC( orbserver<session_i>::thread_entry ), reinterpret_cast<void *>(&server) );
   //ACE_UNUSED_ARG(tidSession);
   server.run();    
   // thrMgr->wait();  // perform a barrier wait until all the threads have shut down.
   
   return 0;
}

static void
parse_args(int argc, ACE_TCHAR *argv[])
{
    while ( --argc ) {
        ++argv;
        if ( *argv[0] == '-' ) {
            if ( strcmp( *argv, "--debug" ) == 0 ) {
                debug_flag = 1;
            } else if ( strcmp( *argv, "--respawn" ) == 0 ) {
                signal_handler::respawn_flag = 1;
            }
        }
    }
}


int
main(int argc, char *argv[])
{
   ACE_Service_Config daemon;

   parse_args( argc, argv );

   signal( SIGHUP, &signal_handler::sigint );
   signal( SIGINT, &signal_handler::sigint );
   signal( SIGQUIT, &signal_handler::sigint );
#if ! defined WIN32
   signal( SIGKILL, &signal_handler::sigint );
#endif
   signal( SIGABRT, &signal_handler::sigint );

   if ( signal_handler::respawn_flag ) {
       signal_handler::pidParent = ACE_OS::getpid();

       do {
          signal_handler::pidChild = ACE_OS::fork();
          if ( signal_handler::pidChild == 0 )
              return run( argc, argv );
          ACE_OS::wait();
      } while ( true );
   } else {
       run( argc, argv );
   }

   ACE_Process_Manager::instance()->close();
   std::cout << "Merci" << std::endl;

   return 0;
}

