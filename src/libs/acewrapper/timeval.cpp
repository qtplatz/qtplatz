//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "timeval.h"
#include <ACE/Time_Value.h>
#include <ACE/High_Res_Timer.h>

namespace acewrapper {
  void gettimeofday(time_t& tv_sec, long& tv_usec) {
        ACE_Time_Value tv = ACE_High_Res_Timer::gettimeofday_hr();
        tv_sec = tv.sec();
        tv_usec = tv.usec();
  }
}
