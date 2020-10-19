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

#include <date/date.h>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <type_traits>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <adportable/date_string.hpp>

template< bool is_system_clock = true >
struct date_time_t {
    template< typename duration_t, typename clock_t, typename time_point_t >
    std::pair< std::time_t, duration_t > to_time_t( time_point_t tp ) {
        std::cout << "-- system_clock -->";
        auto utc = std::chrono::system_clock::to_time_t( std::chrono::time_point_cast< std::chrono::system_clock::duration >( tp ) );
        return { utc, duration_t( std::chrono::time_point_cast< duration_t >( tp ) - date::floor< std::chrono::seconds >( tp ) ) };
    }
};

template<>
template< typename duration_t, typename clock_t, typename time_point_t >
std::pair< std::time_t, duration_t >
date_time_t< false >::to_time_t( time_point_t tp ) {
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
    template< typename duration_t, typename clock_t, typename time_point_t >
    std::string to_iso( time_point_t tp, bool utc_offset = true ) {
#if __cplusplus < 201703L
        std::time_t utc; duration_t duration;
        std::tie( utc, duration )
#else
        auto [utc, duration]
#endif
            = date_time_t< std::is_same< clock_t, std::chrono::system_clock >::value >(). template to_time_t< duration_t, clock_t>( tp );

        auto subseconds = duration.count();
        subseconds_t< num_digits< duration_t::period::den >::value - 1 > subseconds_to_string;

        std::ostringstream o;
        if ( utc_offset ) {
            boost::posix_time::time_duration tz( boost::posix_time::second_clock::local_time() - boost::posix_time::second_clock::universal_time() );
            o << std::put_time( std::localtime(&utc), "%FT%T" ) << subseconds_to_string( subseconds )
              << boost::format( "%c%02d%02d" )
                % (tz.is_negative() ? '-' : '+')
                % boost::date_time::absolute_value( tz.hours() )
                % boost::date_time::absolute_value( tz.minutes() );
        } else {
            o << boost::posix_time::to_iso_extended_string( boost::posix_time::from_time_t( utc ) )
              << subseconds_to_string( subseconds ) << "Z";
        }
        return o.str();
    }
};


int
main()
{
    using this_clock = std::chrono::high_resolution_clock;

    auto tp = this_clock::now();
    auto sys_tp = std::chrono::system_clock::now();
    auto sdy_tp = std::chrono::steady_clock::now();

    std::cout << "us: " << date_time().to_iso< std::chrono::microseconds, std::chrono::system_clock >( sys_tp, true ) << std::endl;
    std::cout << "ms: " << date_time().to_iso< std::chrono::milliseconds, std::chrono::system_clock >( sys_tp, true ) << std::endl;
    std::cout << " s: " << date_time().to_iso< std::chrono::seconds, std::chrono::system_clock >( sys_tp, true ) << std::endl;

    std::cout << "us: " << date_time().to_iso< std::chrono::microseconds, std::chrono::steady_clock >( sdy_tp, true ) << std::endl;
    std::cout << "ms: " << date_time().to_iso< std::chrono::milliseconds, std::chrono::steady_clock >( sdy_tp, true ) << std::endl;
    std::cout << " s: " << date_time().to_iso< std::chrono::seconds, std::chrono::steady_clock >( sdy_tp, true ) << std::endl;

    // std::cout << date_time().to_iso< std::chrono::seconds, std::chrono::system_clock >( sdy_tp, true ) << std::endl;
    std::cout << std::endl;
    std::cout << date_time().to_iso< std::chrono::nanoseconds, std::chrono::steady_clock >( sdy_tp, true ) << std::endl;
    std::cout << date_time().to_iso< std::chrono::nanoseconds, std::chrono::system_clock >( sys_tp, true ) << std::endl;
    std::cout << std::endl;
    // std::cout << date_time().to_iso< std::chrono::nanoseconds, this_clock >( tp, false ) << std::endl;
    std::cout << date_time().to_iso< std::chrono::nanoseconds, std::chrono::steady_clock >( sdy_tp, false ) << std::endl;
    std::cout << date_time().to_iso< std::chrono::nanoseconds, std::chrono::system_clock >( sys_tp, false ) << std::endl;
}
