// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <ace/Task.h>
#include <ace/Reactor_Notification_Strategy.h>
#include <ace/Message_Queue.h>

class Task;
class ACE_Recursive_Thread_Mutex;
template<class T, class M> class ACE_Singleton;

namespace singleton {
   typedef ACE_Singleton< Task, ACE_Recursive_Thread_Mutex > Task;
}

///////////////////////////

class Task : public ACE_Task<ACE_SYNCH> {
      ~Task();
      Task();
public:
      bool initialize( ACE_Reactor * reactor );
      void spawn();
      
      virtual int handle_input( ACE_HANDLE );
      virtual int svc();
      
private:
      ACE_Message_Queue<ACE_SYNCH> msgq_;
      ACE_Reactor_Notification_Strategy * notification_strategy_;
      friend ACE_Singleton< Task, ACE_Recursive_Thread_Mutex>;
};

