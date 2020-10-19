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
        auto utc = std::chrono::system_clock::to_time_t( tp );
        return { utc, duration_t( tp - date::floor< std::chrono::seconds >( tp ) ) };
    }
};

template<>
template< typename duration_t, typename clock_t, typename time_point_t >
std::pair< std::time_t, duration_t >
date_time_t< false >::to_time_t( time_point_t tp ) {
    std::cout << "-- steady_clock -->";
    auto system_tp = std::chrono::system_clock::now();
    auto clock_tp = clock_t::now();
    auto td = system_tp + std::chrono::duration_cast< std::chrono::system_clock::duration>( tp - clock_tp );
    auto utc = std::chrono::system_clock::to_time_t( td );
    auto dur = duration_t( tp - date::floor< std::chrono::seconds >( tp ) );
    return { utc, dur }; // subseconds };
}

struct date_time {
    template< typename duration_t, typename clock_t, typename time_point_t >
    std::string to_iso( time_point_t tp, bool utc_offset = true ) {
        auto [utc, duration]
            = date_time_t< std::is_same< clock_t, std::chrono::system_clock >::value >(). template to_time_t< duration_t, clock_t>( tp );

        auto subseconds = duration.count();

        std::ostringstream o;
        if ( utc_offset ) {
            boost::posix_time::time_duration tz( boost::posix_time::second_clock::local_time() - boost::posix_time::second_clock::universal_time() );
            o << std::put_time( std::localtime(&utc), "%FT%T" ) << "," << std::setw(9) << std::setfill('0') << subseconds
              << boost::format( "%c%02d%02d" )
                % (tz.is_negative() ? '-' : '+')
                % boost::date_time::absolute_value( tz.hours() )
                % boost::date_time::absolute_value( tz.minutes() );
        } else {
            o << boost::posix_time::to_iso_extended_string( boost::posix_time::from_time_t( utc ) )
              << "," << std::setw(9) << std::setfill('0') << subseconds << "Z";
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

    std::cout << date_time().to_iso<std::chrono::nanoseconds, this_clock>( tp, true ) << std::endl;
    std::cout << date_time().to_iso<std::chrono::nanoseconds, std::chrono::system_clock >( sys_tp, true ) << std::endl;

    std::cout << date_time().to_iso<std::chrono::nanoseconds, this_clock>( tp, false ) << std::endl;
    std::cout << date_time().to_iso<std::chrono::nanoseconds, std::chrono::system_clock >( sys_tp, false ) << std::endl;

    // std::cout << date_time_t<false>().to_iso<std::chrono::nanoseconds, this_clock>( tp, true ) << std::endl;
    // std::cout << date_time_t<true>().to_iso<std::chrono::nanoseconds, std::chrono::system_clock >( sys_tp, true ) << std::endl;

    // std::cout << date_time_t<false>().to_iso<std::chrono::nanoseconds, this_clock>( tp, false ) << std::endl;
    // std::cout << date_time_t<true>().to_iso<std::chrono::nanoseconds, std::chrono::system_clock >( sys_tp, false ) << std::endl;

    // std::cout << adportable::date_string::to_iso( epoch, true ) << std::endl;
    // std::cout << adportable::date_string::to_iso( res ) << std::endl;

    // std::cout << adportable::date_string::logformat( since_epoch, true ) << std::endl;
}
