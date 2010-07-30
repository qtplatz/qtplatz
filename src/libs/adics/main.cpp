//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include <adcontroller/adcontroller.h>
#include <adcontroller/signal_handler.h>

#include <ace/Service_Config.h>
#include <ace/Process_Manager.h>
#include <ace/OS.h>
#include <iostream>

static int debug_flag = 0;

#if defined ACE_WIN32
#  if defined _DEBUG
#     pragma comment(lib, "ACEd.lib")
#     pragma comment(lib, "adcontrollerd.lib")
#  else
#     pragma comment(lib, "ACE.lib")
#     pragma comment(lib, "adcontroller.lib")
#  endif
#endif

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

   signal( SIGINT, &signal_handler::sigint );
#if ! defined WIN32
   signal( SIGHUP, &signal_handler::sigint );
   signal( SIGKILL, &signal_handler::sigint );
   signal( SIGQUIT, &signal_handler::sigint );
#endif
   signal( SIGABRT, &signal_handler::sigint );

   if ( signal_handler::respawn_flag ) {
       signal_handler::pidParent = ACE_OS::getpid();

       do {
          signal_handler::pidChild = ACE_OS::fork();
          if ( signal_handler::pidChild == 0 )
			  return adcontroller::run( argc, argv );
          ACE_OS::wait();
      } while ( true );
   } else {
	   adcontroller::run( argc, argv );
	   ACE_Process_Manager::instance()->wait();
   }

   ACE_Process_Manager::instance()->close();
   std::cout << "Merci" << std::endl;

   return 0;
}

