// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef REACTORTHREAD_H
#define REACTORTHREAD_H

#include <ace/Singleton.h>
#include <ace/Reactor.h>
class ACE_Recursive_Thread_Mutex;

namespace acewrapper {

    class ReactorThread {
    private:
        ~ReactorThread();
        ReactorThread();
    public:
        ACE_Reactor * get_reactor();
        static void spawn( ReactorThread * );

    private:
        static void * thread_entry( void * me );
        void run_event_loop();
        ACE_Reactor reactor_;
        friend class ACE_Singleton<ReactorThread, ACE_Recursive_Thread_Mutex>;
    };

    typedef ACE_Singleton<ReactorThread, ACE_Recursive_Thread_Mutex> TheReactorThread;
}

#endif // REACTORTHREAD_H
