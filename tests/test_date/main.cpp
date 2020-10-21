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
#include <adportable/iso8601.hpp>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <type_traits>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <adportable/date_string.hpp>

int
main()
{
    auto tp = std::chrono::high_resolution_clock::now();
    auto sys_tp = std::chrono::system_clock::now();
    auto sdy_tp = std::chrono::steady_clock::now();

    std::cout << "sys us: " << adportable::date_time::to_iso< std::chrono::microseconds >( sys_tp, true ) << std::endl;
    std::cout << "sys ms: " << adportable::date_time::to_iso< std::chrono::milliseconds >( sys_tp, true ) << std::endl;
    std::cout << "sys  s: " << adportable::date_time::to_iso< std::chrono::seconds >( sys_tp, true ) << std::endl;

    std::cout << "sdy us: " << adportable::date_time::to_iso< std::chrono::microseconds >( sdy_tp, true ) << std::endl;
    std::cout << "sdy ms: " << adportable::date_time::to_iso< std::chrono::milliseconds >( sdy_tp, true ) << std::endl;
    std::cout << "sdy  s: " << adportable::date_time::to_iso< std::chrono::seconds >( sdy_tp, true ) << std::endl;

    std::cout << "sys us: " << adportable::date_time::to_iso< std::chrono::microseconds >( sys_tp, false ) << std::endl;
    std::cout << "sys ms: " << adportable::date_time::to_iso< std::chrono::milliseconds >( sys_tp, false ) << std::endl;
    std::cout << "sys s: " << adportable::date_time::to_iso< std::chrono::seconds >( sys_tp, false ) << std::endl;

    std::cout << "sdy us: " << adportable::date_time::to_iso< std::chrono::microseconds >( sdy_tp, false ) << std::endl;
    std::cout << "sdy ms: " << adportable::date_time::to_iso< std::chrono::milliseconds >( sdy_tp, false ) << std::endl;
    std::cout << "sdy  s: " << adportable::date_time::to_iso< std::chrono::seconds >( sdy_tp, false ) << std::endl;
#if 0
    std::cout << std::endl;
    std::cout << adportable::date_time::to_iso< std::chrono::nanoseconds >( sys_tp, true ) << std::endl;
    std::cout << adportable::date_time::to_iso< std::chrono::nanoseconds >( sdy_tp, true ) << std::endl;
    std::cout << adportable::date_time::to_iso< std::chrono::nanoseconds >( tp, true ) << std::endl;

    std::cout << std::endl;
    std::cout << adportable::date_time::to_iso< std::chrono::nanoseconds >( sys_tp, false ) << std::endl;
    std::cout << adportable::date_time::to_iso< std::chrono::nanoseconds >( sdy_tp, false ) << std::endl;
    std::cout << adportable::date_time::to_iso< std::chrono::nanoseconds >( tp, false ) << std::endl;
#endif

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
        auto str = std::string( s );
        if ( auto tp = adportable::iso8601::parse( str.begin(), str.end() ) ) {
            std::cout << str << "\t-->\t";
            std::cout << adportable::date_time::to_iso< std::chrono::nanoseconds >( *tp ) << "\t";
            // using namespace date;
            // std::cout << "date_t: " << date::format( "%FT%TZ", *tp ) << std::endl;
        } else {
            std::cout << str << "\tparse failed\t" << std::endl;
        }
    }
}
