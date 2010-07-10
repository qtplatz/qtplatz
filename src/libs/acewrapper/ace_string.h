// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <string>

class ACE_INET_Addr;

namespace acewrapper {

  class string {
    std::wstring t_;
  public:
    string();
    string( const ACE_INET_Addr& );
    operator const std::wstring& () { return t_; }
  };
  
}

