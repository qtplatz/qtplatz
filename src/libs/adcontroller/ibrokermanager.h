// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include <ace/Singleton.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <boost/smart_ptr.hpp>

class iBroker;
class ACE_Reactor;
class Task;

class IBrokerManager {
private:
    ~IBrokerManager();
    IBrokerManager();
    IBrokerManager( const IBrokerManager& );  /* not defined */

   public:  
       bool initialize();
	   void terminate();

       inline ACE_Recursive_Thread_Mutex& mutex() { return mutex_; }
	   inline ACE_Reactor * reactor() { return reactor_; }

       template<class T> T* get();
       template<> iBroker * get<iBroker>() { return pBroker_; }
      
   private:
       friend class ACE_Singleton<IBrokerManager, ACE_Recursive_Thread_Mutex>;

       ACE_Recursive_Thread_Mutex mutex_;
       iBroker * pBroker_;

       // main contrl server reactor
       ACE_Reactor * reactor_;
	   static void * reactor_thread_entry( void * );

	   // a task
	   boost::shared_ptr< Task > task_;

	   void run_event_loop();
};

typedef ACE_Singleton<IBrokerManager, ACE_Recursive_Thread_Mutex> iBrokerManager;
