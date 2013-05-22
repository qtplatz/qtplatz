/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "signal_handler.hpp"
#include <iostream>
#include <ace/Sched_Params.h>
#include <ace/Thread_Manager.h>
#include <ace/Process_Manager.h>
#include <ace/OS_NS_unistd.h>
#include <ace/OS_NS_sys_wait.h>
#include "adcontroller.hpp"

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
       // static int inProgress = 0;
       std::cerr << "################ abort " << ACE_OS::getpid() << "( signal=" << num << ") " << "##################" << std::endl;
       adController::_abort_server();
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

