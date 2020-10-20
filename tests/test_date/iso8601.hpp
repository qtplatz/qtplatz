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
#include <boost/optional.hpp>
#include <boost/fusion/include/map.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <type_traits>
#include <utility>

namespace iso8601 {

    //------------
    template< uint64_t N > struct n_power_of_ten {
        enum { value = 10 * n_power_of_ten< N - 1 >::value };
    };
    template<> struct n_power_of_ten<0> {
        enum { value = 1 };
    };

    //------------
    template< size_t ... > struct duration_type;

    template< size_t arg > struct duration_type< arg > {
        auto operator()( size_t N, int64_t subseconds ) const {
            return std::chrono::nanoseconds{};
        }
    };

    template< size_t first, size_t ... args > struct duration_type< first, args ...> {
        auto operator()(size_t N, int64_t subseconds ) const {
            if ( N == first ) {
                // std::cout << "===============> duration_type<" << n_power_of_ten< first >::value << ">" << std::endl;
                auto dur = std::chrono::duration< int64_t, std::ratio< 1, n_power_of_ten< first >::value > >{subseconds}; // { subseconds };
                return std::chrono::duration_cast< std::chrono::nanoseconds >( dur );
            } else
                return duration_type< args ... >()( N, subseconds );
        }
    };

    //----------------------------- parser --------------------------------
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;
    namespace phoenix = boost::phoenix;
    using qi::_1;
    using phoenix::ref;
    using ascii::space;

    typedef std::tuple< int, int, int
                        , int, int, int, std::string
                        , char, int > date_time_type;

    template <typename Iterator>
    boost::optional< std::chrono::system_clock::time_point > parse( Iterator first, Iterator last, date_time_type& dt )
    {
        dt = date_time_type{};

        auto tzf = [&](auto& ctx){ std::get< 7 >(dt) = ctx; };
        auto subs = [&](auto& ctx){ std::get< 6 >(dt) += ctx; };

        bool r = qi::phrase_parse(
            first
            , last
            , (
                qi::int_[ ref( std::get<0>(dt) ) = _1 ]
                >> '-' >> qi::int_[ref( std::get<1>(dt) ) = _1 ]
                >> '-' >> qi::int_[ref( std::get<2>(dt) ) = _1 ]
                >> 'T' >> qi::int_[ref( std::get<3>(dt) ) = _1 ]
                >> ':' >> qi::int_[ref( std::get<4>(dt) ) = _1 ]
                >> ':' >> qi::int_[ref( std::get<5>(dt) ) = _1 ]
                >> -(
                    (qi::char_('.')|qi::char_(',')) >> +qi::digit [ subs ]
                    )
                >> ( qi::char_('Z') [ tzf ]
                     |( qi::char_("+-") [ tzf ] >> qi::int_[ ref( std::get<8>(dt) ) = qi::_1 ] )
                    )
                )
            , space);

        if ( r ) {
            duration_type< 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 > dlist;  // seconds .. picoseconds
            auto subseconds = dlist( std::get<6>(dt).size(), std::strtoull( std::get<6>(dt).c_str(), nullptr, 10 ) ); // (subseconds (ns))

            std::cout << "count: " << subseconds.count() << std::endl;
            std::tm tm;
            tm.tm_year = std::get<0>(dt) - 1900;
            tm.tm_mon  = std::get<1>(dt) - 1;
            tm.tm_mday = std::get<2>(dt);
            tm.tm_hour = std::get<3>(dt);
            tm.tm_min  = std::get<4>(dt);
            tm.tm_sec  = std::get<5>(dt);
            std::mktime(&tm);

            std::cout << "100 months ago was " << std::put_time(&tm, "%c %Z")
                      << " and DST was " << (tm.tm_isdst ? "in effect" : "not in effect") << '\n';

            using namespace date;
            using namespace std::chrono;

            // auto tp = sys_time< std::chrono::system_clock::duration >( year{ std::get<0>(dt) }/std::get<1>(dt)/std::get<2>(dt) + 0h );
            return std::chrono::system_clock::time_point{};
        }
        return boost::none;
    }
}
