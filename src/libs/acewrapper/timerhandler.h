// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef TIMERHANDLER_H
#define TIMERHANDLER_H

#include <ace/Event_Handler.h>

class ACE_Semaphore;
//class ACE_Thread_Mutex;
//template<class T> class ACE_Condition;

namespace acewrapper {

    class TimerHandler {
    public:
        ~TimerHandler();
        TimerHandler();
        ACE_HANDLE get_handle() const { return 0; /* meanless for timer */ }
        void cancel( ACE_Reactor*, ACE_Event_Handler *);
        int signal();
        int wait();
    private:
        ACE_Semaphore * sema_;
   };

   ////////////////
   template<class T> class TimerReceiver : public T
					                     , public TimerHandler {
   public:
	 TimerReceiver() {}
	 int handle_timeout( const ACE_Time_Value& tv, const void * arg) { return T::handle_timeout(tv, arg); }
     int handle_close( ACE_HANDLE h, ACE_Reactor_Mask mask ) { signal(); return T::handle_close( h, mask ); }
  };

}

#endif // TIMERHANDLER_H
