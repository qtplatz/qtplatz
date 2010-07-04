// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef TIMEVAL_H
#define TIMEVAL_H

#include <time.h>

namespace acewrapper {

  void gettimeofday(time_t& tv_sec, long& tv_usec);

}

#endif // TIMEVAL_H
