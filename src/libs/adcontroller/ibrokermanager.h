// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include <ace/Singleton.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <boost/smart_ptr.hpp>

class ACE_Reactor;

namespace acewrapper {
	class ReactorThread;
    template<class T> class EventHandler;
    class TimerHandler;
    template<class T> class TimerReceiver;
}

namespace adcontroller {

	class iBroker;

    namespace internal {
        class TimeReceiver;
    }
    
    class IBrokerManager {
    private:
        ~IBrokerManager();
        IBrokerManager();
        IBrokerManager( const IBrokerManager& );  /* not defined */
        
    public:  
        bool manager_initialize();
        void manager_terminate();
        
        inline ACE_Recursive_Thread_Mutex& mutex() { return mutex_; }
        ACE_Reactor * reactor();
        
        template<class T> T* get();
        template<> iBroker * get<iBroker>() { return pBroker_; }
        
    private:
        friend class ACE_Singleton<IBrokerManager, ACE_Recursive_Thread_Mutex>;
        friend class internal::TimeReceiver;
        int handle_timeout( const ACE_Time_Value&, const void * );
        
        ACE_Recursive_Thread_Mutex mutex_;
        iBroker * pBroker_;
        acewrapper::ReactorThread * reactor_thread_;    
        acewrapper::EventHandler< acewrapper::TimerReceiver<internal::TimeReceiver> > * timerHandler_;
    };

	namespace singleton {
		typedef ACE_Singleton<adcontroller::IBrokerManager, ACE_Recursive_Thread_Mutex> iBrokerManager;
	}
}


