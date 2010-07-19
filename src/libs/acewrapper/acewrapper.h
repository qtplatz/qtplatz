// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef ACEWRAPPER_H
#define ACEWRAPPER_H

// #include <ace/Singleton.h>
template<class T, class X> class ACE_Singleton;
class ACE_Recursive_Thread_Mutex;

namespace acewrapper {

  class instance_manager {
  private:
      ~instance_manager();
      instance_manager();
      void initialize_i();
      void finalize_i();
  public:
      static void initialize();
      static void finalize();
      friend class ACE_Singleton<instance_manager, ACE_Recursive_Thread_Mutex>;
  };
  typedef ACE_Singleton<instance_manager, ACE_Recursive_Thread_Mutex> instance_manager_t;
}

#endif // ACEWRAPPER_H
