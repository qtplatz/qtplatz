// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef REACTORTHREAD_H
#define REACTORTHREAD_H

class ACE_Semaphore;
class ACE_Reactor;
template<class T, class X> class ACE_Singleton;

namespace acewrapper {

    class ReactorThread {
	public:
        ~ReactorThread();
        ReactorThread();

        ACE_Reactor * get_reactor();
        void terminate();
        bool spawn();

        static void spawn( ReactorThread * );

    private:
        static void * thread_entry( void * me );
        void run_event_loop();
        ACE_Reactor * reactor_;
		ACE_Semaphore * sema_;
    };
}

#endif // REACTORTHREAD_H
