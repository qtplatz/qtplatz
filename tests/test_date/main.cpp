// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2020 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@qtplatz.com
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

#include "iso8601.hpp"

#include <date/date.h>
#include <adportable/date_time.hpp>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <type_traits>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <adportable/date_string.hpp>


namespace test {

    template< bool is_system_clock = true >
    struct date_time_t {
        template< typename duration_t, typename time_point_t >
        std::pair< std::time_t, duration_t > to_time_t( time_point_t tp ) {
            typedef typename decltype( tp )::clock clock_t;
            std::cout << "-- system_clock -->";
            auto utc = std::chrono::system_clock::to_time_t( std::chrono::time_point_cast< std::chrono::system_clock::duration >( tp ) );
            return { utc, duration_t( std::chrono::time_point_cast< duration_t >( tp ) - date::floor< std::chrono::seconds >( tp ) ) };
        }
    };

    template<>
    template< typename duration_t, typename time_point_t >
    std::pair< std::time_t, duration_t >
    date_time_t< false >::to_time_t( time_point_t tp ) {
        typedef typename decltype( tp )::clock clock_t;
        std::cout << "-- steady_clock -->";
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

    template< int N >
    struct subseconds_t {
        std::string operator()( uint64_t value ) const {
            std::ostringstream o;
            o << "," << std::setw(N) << std::setfill('0') << value;
            return o.str();
        }
    };

    template<> std::string subseconds_t<0>::operator()( uint64_t ) const {
        return std::string();
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

            subseconds_t< num_digits< duration_t::period::den >::value - 1 > subseconds_to_string;

            std::ostringstream o;
            if ( utc_offset ) {
                auto tz( boost::posix_time::second_clock::local_time() - boost::posix_time::second_clock::universal_time() );
                o << std::put_time( std::localtime(&utc), "%FT%T" ) << subseconds_to_string( subseconds.count() )
                  << boost::format( "%c%02d%02d" )
                    % (tz.is_negative() ? '-' : '+')
                    % boost::date_time::absolute_value( tz.hours() )
                    % boost::date_time::absolute_value( tz.minutes() );
            } else {
                o << boost::posix_time::to_iso_extended_string( boost::posix_time::from_time_t( utc ) )
                  << subseconds_to_string( subseconds.count() ) << "Z";
            }
            return o.str();
        }
    };
} // namespace test

int
main()
{
    auto tp = std::chrono::high_resolution_clock::now();
    auto sys_tp = std::chrono::system_clock::now();
    auto sdy_tp = std::chrono::steady_clock::now();

    std::cout << "us: " << adportable::date_time::to_iso< std::chrono::microseconds >( sys_tp, true ) << std::endl;
    std::cout << "ms: " << adportable::date_time::to_iso< std::chrono::milliseconds >( sys_tp, true ) << std::endl;
    std::cout << " s: " << adportable::date_time::to_iso< std::chrono::seconds >( sys_tp, true ) << std::endl;

    std::cout << "us: " << adportable::date_time::to_iso< std::chrono::microseconds >( sdy_tp, true ) << std::endl;
    std::cout << "ms: " << adportable::date_time::to_iso< std::chrono::milliseconds >( sdy_tp, true ) << std::endl;
    std::cout << " s: " << adportable::date_time::to_iso< std::chrono::seconds >( sdy_tp, true ) << std::endl;

    std::cout << std::endl;
    std::cout << adportable::date_time::to_iso< std::chrono::nanoseconds >( tp, true ) << std::endl;
    std::cout << adportable::date_time::to_iso< std::chrono::nanoseconds >( sdy_tp, true ) << std::endl;
    std::cout << adportable::date_time::to_iso< std::chrono::nanoseconds >( sys_tp, true ) << std::endl;

    std::cout << std::endl;
    std::cout << adportable::date_time::to_iso< std::chrono::nanoseconds >( tp, false ) << std::endl;
    std::cout << adportable::date_time::to_iso< std::chrono::nanoseconds >( sdy_tp, false ) << std::endl;
    std::cout << adportable::date_time::to_iso< std::chrono::nanoseconds >( sys_tp, false ) << std::endl;

    //--------------
    std::cout << "parser test" << std::endl;
    auto str = adportable::date_time::to_iso< std::chrono::nanoseconds >( tp, false );

    for ( auto s: { "2020-10-20T09:31:54+0900"
                , "2020-10-20T09:31:54,125+0900"
                , "2020-10-20T09:31:54,125197+0900"
                , "2020-10-20T09:31:54,125197267+0900"
                , "2020-10-20T09:31:54.125197267-0800"
                , "2020-10-19T23:24:16,497177778Z"
                , "2020-10-19T23:24:16.000077778Z" } ) {
        iso8601::date_time_type dt;
        auto str = std::string( s );
        if ( iso8601::parse( str.begin(), str.end(), dt ) ) {
            std::cout << str << "\tok -->\t";
            std::cout << std::get<0>(dt) << "-" << std::get<1>(dt) << "-" << std::get<2>(dt) << "T";
            std::cout << std::get<3>(dt) << ":" << std::get<4>(dt) << ":" << std::get<5>(dt) << "," << std::get<6>(dt)
                      << " " << std::get<7>(dt) << ", " << std::get<8>(dt)
                      << std::endl;
            // std::cout << tm.tm_year << "-" << tm.tm_mon << "-" << tm.tm_mday << "\t";
            // std::cout << tm.tm_hour << "-" << tm.tm_min << "-" << tm.tm_sec << std::endl;
        } else {
            std::cout << str << "\tparse failed\t";
            std::cout << std::get<0>(dt) << "-" << std::get<1>(dt) << "-" << std::get<2>(dt) << "T";
            std::cout << std::get<3>(dt) << ":" << std::get<4>(dt) << ":" << std::get<5>(dt) << "," << std::get<6>(dt)
                      << " " << std::get<7>(dt) << ", " << std::get<8>(dt)
                      << std::endl;

        }
    }
}
