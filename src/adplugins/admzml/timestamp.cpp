// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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

#include "timestamp.hpp"
#include <adportable/debug.hpp>
#include <adportable/date_time.hpp>
#include <adportable/iso8601.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/json.hpp>
#include <ctime>
#include <format>

namespace adnetcdf {

    struct utc {
        static long offset() {
            using namespace boost::posix_time;
            using namespace boost::gregorian;
            constexpr const ptime utc( boost::gregorian::date(1970, Jan, 1), time_duration(0, 0, 0, 0) );
            boost::posix_time::time_duration td = boost::date_time::c_local_adjustor<ptime>::utc_to_local( utc ) - utc;
            return td.hours() * 3600 + td.minutes() * 60 + td.seconds();
        }
    };

    std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds >
    time_stamp_parser::operator()( const std::string& timestamp, bool ignore_tz ) const
    {
        std::istringstream is( timestamp );
        std::tuple< char, int > tz = {0,0}; //std::string tz;
        std::tm t = {};
        is >> std::get_time(&t, "%Y%m%d%H%M%S"); // 20240112113323+0000
        if ( not is.eof() )
            is >> std::get<0>(tz) >> std::get<1>(tz);

        const int sign = std::get<0>(tz) == '-' ? -1 : std::get<0>(tz) == '+' ? 1 : 0;
        int tzoffs = sign * ( std::get<1>(tz) / 100 ) * 3600 + ( (std::get<1>(tz) % 100 ) * 60 );

        std::time_t utc = ( ignore_tz && tzoffs == 0 )
            ? std::mktime(&t)
            : ( std::mktime(&t) - utc::offset() ) + tzoffs;

        return std::chrono::system_clock::time_point{} + std::chrono::seconds( utc );
    }

    std::string
    iso8601::operator()( const time_point_type& tp ) const
    {
        return adportable::date_time::to_iso<std::chrono::milliseconds>( tp );
    }

}
