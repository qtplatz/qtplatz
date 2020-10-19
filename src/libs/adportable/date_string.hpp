/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#pragma once

#include "adportable_global.h"
#include <date/date.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <chrono>
#include <ctime>
#include <iostream>
#include <string>
#include <type_traits>

namespace boost { namespace gregorian { class date; } }

namespace adportable {

    template< bool is_system_clock = true >
    struct date_time_t {
        template< typename duration_t, typename clock_t, typename time_point_t >
        std::pair< std::time_t, duration_t > to_time_t( time_point_t tp ) {
            // std::cout << "-- system_clock -->";
            auto utc = std::chrono::system_clock::to_time_t( tp );
            return { utc, duration_t( tp - date::floor< std::chrono::seconds >( tp ) ) };
        }
    };

    template<>
    template< typename duration_t, typename clock_t, typename time_point_t >
    std::pair< std::time_t, duration_t >
    date_time_t< false >::to_time_t( time_point_t tp ) {
        // std::cout << "-- steady_clock -->";
        auto tt = std::chrono::time_point_cast< duration_t >( std::chrono::system_clock::now() ) + ( tp - clock_t::now() );
        auto utc = std::chrono::system_clock::to_time_t( std::chrono::time_point_cast< std::chrono::system_clock::duration >( tt ) );
        return { utc, tt - date::floor< std::chrono::seconds >( tt ) };
    }

    struct date_time {
        template< typename duration_t, typename clock_t, typename time_point_t >
        std::string to_iso8601( time_point_t tp, bool utc_offset = true ) {
#if __cplusplus < 201703L
            std::time_t utc; duration_t duration;
            std::tie( utc, duration )
#else
            auto [utc, duration]
#endif
                  = date_time_t< std::is_same< clock_t, std::chrono::system_clock >::value >(). template to_time_t< duration_t, clock_t>( tp );

            auto subseconds = duration.count();
            size_t n = 1;
            while ( duration_t::period::den / n++ )
                ;

            std::ostringstream o;
            if ( utc_offset ) {
                boost::posix_time::time_duration tz( boost::posix_time::second_clock::local_time() - boost::posix_time::second_clock::universal_time() );
                o << std::put_time( std::localtime(&utc), "%FT%T" ) << "," << std::setw(n) << std::setfill('0') << subseconds
                  << boost::format( "%c%02d%02d" )
                    % (tz.is_negative() ? '-' : '+')
                    % boost::date_time::absolute_value( tz.hours() )
                    % boost::date_time::absolute_value( tz.minutes() );
            } else {
                o << boost::posix_time::to_iso_extended_string( boost::posix_time::from_time_t( utc ) )
                  << "," << std::setw(n) << std::setfill('0') << subseconds << "Z";
            }
            return o.str();
        }
    };

    ////////////////

    class ADPORTABLESHARED_EXPORT date_string {
    public:
        static std::string string( const boost::gregorian::date& dt, const char * fmt = "%Y-%m-%d" );
        static std::wstring wstring( const boost::gregorian::date& dt, const wchar_t * fmt = L"%Y-%m-%d" );
        static std::string utc_to_localtime_string( time_t utc, unsigned usec, bool add_utc_offset = false );
        static std::string logformat( const std::chrono::system_clock::time_point& tp, bool add_utc_offset = false );

        // template< typename duration_t = std::chrono::microseconds >
        // std::string to_iso( std::chrono::system_clock_time_point&& tp, bool utc_offset = true ) {
        //     std::time_t utc = std::chrono::to_time_t( tp );
        //     auto subseconcs = duration_t( tp.time_since_epoch() ).count() - utc;
        //     if ( utc_offset ) {
        //         std::ostringstream o;
        //         o << std::put_time( std::localtime(&utc), "%FT%T" ) << "." << subseconds;
        //     }
        //     return std::put_time( std::localtime(&utc), "%FT%T" );
        // }
    };

}
