//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "timeval.h"
#include <ACE/Time_Value.h>
#include <ACE/High_Res_Timer.h>
#include <sstream>
#include <string>
#include <time.h>
#include <iomanip>

namespace acewrapper {

  void gettimeofday(time_t& tv_sec, long& tv_usec)
  {
        ACE_Time_Value tv = ACE_High_Res_Timer::gettimeofday_hr();
        tv_sec = tv.sec();
        tv_usec = tv.usec();
  }
  
  std::string to_string( time_t tm ) 
  {
      char tbuf[64];
	  ctime_s(tbuf, sizeof(tbuf), &tm);
	  *::strrchr( tbuf, '\n' ) = '\0';
	  return tbuf;
  }

  std::string to_string( const ACE_Time_Value& tv ) 
  {
	  std::ostringstream o;
      o << to_string( tv.sec() );
	  o << " " << std::fixed << std::setprecision(3) << double ( tv.usec() / 1000 );
	  return o.str();
  }
}
