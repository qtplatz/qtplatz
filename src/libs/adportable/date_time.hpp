/**************************************************************************
** Copyright (C) 2020 MS-Cheminformatics LLC
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

namespace adportable {

    struct ADPORTABLESHARED_EXPORT subseconds {
        static std::string to_str( int64_t, size_t n );
    };

    template< bool is_system_clock = true >
    struct date_time_t {
        template< typename duration_t, typename time_point_t >
        std::pair< std::time_t, duration_t > to_time_t( time_point_t tp ) {
            // std::cout << "-- system_clock -->";
            auto utc = std::chrono::system_clock::to_time_t( std::chrono::time_point_cast< std::chrono::system_clock::duration >( tp ) );
            return { utc, duration_t( std::chrono::time_point_cast< duration_t >( tp ) - date::floor< std::chrono::seconds >( tp ) ) };
        }
    };

    template<>
    template< typename duration_t, typename time_point_t >
    std::pair< std::time_t, duration_t >
    date_time_t< false >::to_time_t( time_point_t tp ) {
        typedef typename decltype( tp )::clock clock_t;
        // std::cout << "-- steady_clock -->";
        auto tt = std::chrono::time_point_cast< duration_t >( std::chrono::system_clock::now() ) + ( tp - clock_t::now() );
        auto utc = std::chrono::system_clock::to_time_t( std::chrono::time_point_cast< std::chrono::system_clock::duration >( tt ) );
        return { utc, std::chrono::duration_cast< duration_t >( tt - date::floor< std::chrono::seconds >( tt ) ) };
    }

    template< uint64_t N > struct num_digits {
        enum { value = 1 + num_digits< N / 10 >::value };
    };

    template<> struct num_digits<0> {
        enum { value = 0 };
    };

    struct date_time {

        template< typename duration_t, typename time_point_t >
        static std::string to_iso( time_point_t tp, bool utc_offset = true ) {
            typedef typename decltype( tp )::clock clock_t;
#if __cplusplus < 201703L
            std::time_t utc; duration_t subseconds;
            std::tie( utc, subseconds )
#else
            auto [utc, subseconds]
#endif
                  = date_time_t< std::is_same< clock_t, std::chrono::system_clock >::value >(). template to_time_t< duration_t >( tp );

            // subseconds_t< num_digits< duration_t::period::den >::value - 1 > subseconds_to_string;
            constexpr size_t N = num_digits< duration_t::period::den >::value - 1;

            std::ostringstream o;
            if ( utc_offset ) {
                auto tz( boost::posix_time::second_clock::local_time() - boost::posix_time::second_clock::universal_time() );
                o << std::put_time( std::localtime(&utc), "%FT%T" ) << subseconds::to_str( subseconds.count(), N )
                  << boost::format( "%c%02d%02d" )
                    % (tz.is_negative() ? '-' : '+')
                    % boost::date_time::absolute_value( tz.hours() )
                    % boost::date_time::absolute_value( tz.minutes() );
            } else {
                auto syst = std::chrono::time_point_cast< duration_t >( std::chrono::system_clock::from_time_t( utc ) ) + duration_t( subseconds );
                o << date::format("%FT%TZ", syst );
            }
            return o.str();
        }
    };
}
