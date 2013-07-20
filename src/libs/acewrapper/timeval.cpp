// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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
		static const boost::posix_time::ptime epock( boost::gregorian::date( 1970, 1, 1 ) );
		boost::posix_time::time_duration d( boost::posix_time::microsec_clock::local_time() - epock );
        tv_sec  = d.total_seconds();
        tv_usec = static_cast< long >( d.fractional_seconds() );
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

    std::wstring to_wstring( unsigned long long sec, unsigned long usec )
    {
        return to_wstring( to_string( sec, usec ) );
    }

}
