// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef TIMEVAL_H
#define TIMEVAL_H

#include <time.h>
#include <string>

class ACE_Time_Value;

namespace acewrapper {

  void gettimeofday(time_t& tv_sec, long& tv_usec);
  std::string to_string( time_t );
  std::string to_string( const ACE_Time_Value& );
  std::string to_string( unsigned long long sec, unsigned long usec );
  std::wstring to_wstring( unsigned long long sec, unsigned long usec );
}

#endif // TIMEVAL_H
