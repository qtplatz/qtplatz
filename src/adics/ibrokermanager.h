// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include <ace/Singleton.h>
#include <ace/Recursive_Thread_Mutex.h>

class iBroker;

class IBrokerManager {
private:
    ~IBrokerManager();
    IBrokerManager();
    IBrokerManager( const IBrokerManager& );  /* not defined */

   public:  
       inline ACE_Recursive_Thread_Mutex& mutex() { return mutex_; }
       template<class T> T* get();
       template<> iBroker * get<iBroker>() { return pBroker_; }
      
   private:
       friend class ACE_Singleton<IBrokerManager, ACE_Recursive_Thread_Mutex>;

       ACE_Recursive_Thread_Mutex mutex_;
       iBroker * pBroker_;
};

typedef ACE_Singleton<IBrokerManager, ACE_Recursive_Thread_Mutex> iBrokerManager;
