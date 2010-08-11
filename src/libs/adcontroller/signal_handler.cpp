//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "signal_handler.h"
#include <iostream>
#pragma warning ( disable : 4996 )
#include <ace/Sched_Params.h>
#include <ace/Thread_Manager.h>
#include <ace/Process_Manager.h>
#include <ace/OS.h>
#pragma warning ( default : 4996 )

#include "adcontroller.h"

int signal_handler::pidChild = 0;
int signal_handler::pidParent = 0;
int signal_handler::respawn_flag = 0;


void
signal_handler::sigint( int num )
{
   std::cout << "signal_handler::sigint num=" << num << std::endl;
   
   ACE_Sched_Params sched_params( ACE_SCHED_OTHER, 
				  ACE_Sched_Params::priority_min( ACE_SCHED_OTHER ),
				  ACE_SCOPE_PROCESS );
   
   if ( ACE_OS::sched_params(sched_params) == -1 ) {
      if ( errno == EPERM || errno == ENOTSUP ) 
	 ACE_DEBUG((LM_DEBUG, "Warning: user's not superuser, so we'll run in the theme-shared class\n"));
   } else {
      ACE_DEBUG((LM_DEBUG, "priority set lower success\n"));
   }
   
   if ( pidChild == 0 ) {
      static int inProgress = 0;
      std::cerr << "################ abort " << ACE_OS::getpid() << "( signal=" << num << ") " << "##################" << std::endl;
	  adController::abort_server();
   } else {
      for ( int i = 0; i < 3; ++i ) {
          std::cerr << "################ kill( " << pidChild << ") ##################" << std::endl;
          ACE_OS::kill( pidChild, SIGHUP );
          ACE_OS::sleep(1);
      }
      exit(0);
      ACE_OS::wait();		
   }
}

