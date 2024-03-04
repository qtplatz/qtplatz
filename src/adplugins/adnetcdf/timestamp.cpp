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
#include <boost/json.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <ctime>
#include <format>

namespace adnetcdf {

    std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds >
    time_stamp_parser::operator()( const boost::json::value& jv, const std::string& path ) const
    {
        boost::system::error_code ec;
        if ( auto value = jv.find_pointer( path, ec ) ) {
            if ( value->is_string() ) {
                std::istringstream is( std::string( value->as_string() ) );
                std::string tz;
                std::tm t;
                is >> std::get_time(&t, "%Y%m%d%H%M%S") >> tz; // 20240112113323+0000 2024-01-12-11:33:23+0000
                std::time_t time = std::mktime( &t );
                ADDEBUG() << std::make_tuple( t.tm_isdst, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec );
                ADDEBUG() << std::string( value->as_string() );
                auto tp = std::chrono::system_clock::time_point{} + std::chrono::seconds( time );
                return tp;
                // 2024-03-01 12:48:02+0000
            }
        }
        return {};
    }


    std::string
    iso8601::operator()( time_point_type&& tp ) const
    {
        return adportable::date_time::to_iso<std::chrono::nanoseconds>( tp );
    }

}
