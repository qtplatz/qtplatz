// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <ace/INET_Addr.h>

namespace acewrapper {

  class Callback {
  public:
    virtual void operator()(const char *, ssize_t, const ACE_INET_Addr& ) { }
  };
  
}

