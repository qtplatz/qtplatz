// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/
#include <compiler/disable_unused_parameter.h>

#include "timeval.hpp"
#include <sstream>
#include <string>
#include <time.h>
#include <iomanip>
#include <boost/date_time/posix_time/posix_time.hpp>

#  include <ace/Time_Value.h>
#  include <ace/High_Res_Timer.h>
#  include <ace/OS_NS_sys_time.h>


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
        // ACE_Time_Value tv = ACE_High_Res_Timer::gettimeofday_hr();
        ACE_Time_Value tv = ACE_OS::gettimeofday();
        tv_sec = tv.sec();
        tv_usec = tv.usec();
    }
  
    std::string to_string( time_t tm ) 
    {
	boost::posix_time::ptime pt = boost::posix_time::from_time_t( tm );
	return boost::posix_time::to_simple_string( pt );
    }

    std::string to_string( unsigned long long sec, unsigned long usec )
    {
        std::ostringstream o;
        o << to_string( time_t(sec) ) << " " << std::fixed << std::setw(7) << std::setfill('0') << std::setprecision(3) << double( usec ) / 1000.0;
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
