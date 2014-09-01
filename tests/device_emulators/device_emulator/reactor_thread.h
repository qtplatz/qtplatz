// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <acewrapper/reactorthread.h>
class ACE_Recursive_Thread_Mutex;
template<class T, class Mutex> class ACE_Singlegon;

namespace acewrapper {
    class ReactorThread;

    namespace singleton {
        typedef ACE_Singleton< acewrapper::ReactorThread, ACE_Recursive_Thread_Mutex > ReactorThread;
    }
}
