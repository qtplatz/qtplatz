// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <boost/noncopyable.hpp>
#include <ace/Event_Handler.h>

namespace acewrapper {

    template<class T> class EventHandler : public T
                                         , public ACE_Event_Handler {
    public:
        EventHandler() {}
        EventHandler(const T& t) : T(t) {}

        virtual ACE_HANDLE get_handle() const { return T::get_handle(); }
        virtual int handle_input(ACE_HANDLE h) { return T::handle_input(h); }
        virtual int handle_timeout( const ACE_Time_Value& tv, const void * arg = 0 ) {
            return T::handle_timeout(tv, arg);
        }
        virtual int handle_close( ACE_HANDLE handle, ACE_Reactor_Mask close_mask ) {
            return T::handle_close( handle, close_mask );
        }
    };

}


