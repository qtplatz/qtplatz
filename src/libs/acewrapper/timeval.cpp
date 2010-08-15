//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "timeval.h"
# pragma warning (disable: 4996)
#  include <ACE/Time_Value.h>
#  include <ACE/High_Res_Timer.h>
# pragma warning (default: 4996)
#include <sstream>
#include <string>
#include <time.h>
#include <iomanip>

namespace acewrapper {

    // CAUTION:  do not use this function for general string variables
    // because this is not locale safe.  Only it works with Latein-1 text
    static std::wstring to_wstring( const std::string& sstr )
    {
        std::wstring wstr( sstr.size() + 1, L'\0' );
        std::copy( sstr.begin(), sstr.end(), wstr.begin() );
        return wstr;
    }

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
        return std::string( tbuf );
    }

    std::string to_string( unsigned long long sec, unsigned long usec )
    {
        std::ostringstream o;
        o << to_string( time_t(sec) ) << " " << std::fixed << std::setprecision(3) << double( usec ) / 1000.0;
        std::string ret = o.str();
        return ret;
    }

    std::string to_string( const ACE_Time_Value& tv ) 
    {
        return to_string( tv.sec(), tv.usec() );
    }

    std::wstring to_wstring( const ACE_Time_Value& tv )
    {
        return to_wstring( to_string( tv ) );
    }

    std::wstring to_wstring( unsigned long long sec, unsigned long usec )
    {
        return to_wstring( to_string( sec, usec ) );
    }

}
