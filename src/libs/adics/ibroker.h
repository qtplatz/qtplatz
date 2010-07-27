// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <ace/Recursive_Thread_Mutex.h>

class iBroker {
      ~iBroker();
      iBroker();
 public:  
      inline ACE_Recursive_Thread_Mutex& mutex() { return mutex_; }
      void reset_clock();
      bool initialize();
  
 private:
     friend class IBrokerManager;

     ACE_Recursive_Thread_Mutex mutex_;
};

