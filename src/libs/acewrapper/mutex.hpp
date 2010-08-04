// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#pragma once

class ACE_Recursive_Thread_Mutex;

namespace acewrapper {

  template<class Mutex = ACE_Recursive_Thread_Mutex>
  class scoped_mutex_t {
    Mutex & mutex_;
  public:
	scoped_mutex_t(Mutex& t) : mutex_(t) {
	  mutex_.acquire();
    }
    virtual ~scoped_mutex_t() {
	  mutex_.release();
    }
  };
  
  template<class Mutex = ACE_Recursive_Thread_Mutex>
  class scoped_acquired_mutex_t {
    Mutex & mutex_;
  public:
    scoped_acquired_mutex_t(Mutex& t) : mutex_(t) {
	  // already acquired
    }
    virtual ~scoped_acquired_mutex_t() {
	  mutex_.release();
    }
  };

}


