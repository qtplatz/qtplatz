// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2018 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#include "boost_ptree.hpp"
#include "qt5_json.hpp"
#include "boost_json.hpp"
#if HAVE_NLOHMANN_JSON
#include "nlohmann_json.hpp"
#endif
#if HAVE_RAPIDJSON_JSON
#include "rapidjson_json.hpp"
#endif
#include "data.hpp"
#include <boost/format.hpp>
#include <chrono>
#include <iostream>
#include <fstream>

constexpr const int reference = 2;
data global_data;

std::string
load_data()
{
    std::ifstream inf( DATAFILE ); // this was created by pteee (not comformed json)

    auto ptree_str = std::string((std::istreambuf_iterator<char>(inf)),  (std::istreambuf_iterator<char>()));
    boost_ptree pt;
    pt.parse( ptree_str );

    pt.map( global_data );
    auto json = boost_json().make_json( global_data ); // make conformed json string

    return json;
}

template< typename T >
struct json_parser {
    double static parse( data& data, const std::string& json_string ) {
        T parser;
        auto tp0 = std::chrono::steady_clock::now();
        if ( parser.parse( json_string ) ) {
            double dur = double( std::chrono::nanoseconds( std::chrono::steady_clock::now() - tp0 ).count() ) / 1e3;
            parser.map( data );
            return dur;
        }
        return 0;
    }

    // json write ( c++ class -> json.stringify )
    double static json_write( const data& data )  {
        auto tp0 = std::chrono::steady_clock::now();
        auto json = T::make_json( data );
        return double( std::chrono::nanoseconds( std::chrono::steady_clock::now() - tp0 ).count() ) / 1e3;
    }

    // json read ( json string -> c++ class )
    double static json_read( data& data, const std::string& json_string ) {
        T parser;
        auto tp0 = std::chrono::steady_clock::now();
        if ( parser.parse( json_string ) ) {
            parser.map( data );
            return double( std::chrono::nanoseconds( std::chrono::steady_clock::now() - tp0 ).count() ) / 1e3;
        }
        return 0;
    }

    static std::string stringify( const data& data ) {
        return T::make_json( data );
    }

    std::string stringify( const std::string& json_string ) {
        T parser;
        parser.parse( json_string );
        return parser.stringify();
    }
};


int
main()
{
    const std::string json_string = load_data();

    std::ofstream null( "/dev/null" );

    std::array< double, 5 > durations = { 0 };

    for ( size_t i = 0; i < 100; ++i ) {
        {   data data;
            durations[ 0 ] += json_parser< boost_ptree >::parse( data, json_string );
        }
        {   data data;
            durations[ 1 ] += json_parser< qt5_json >::parse( data, json_string );
        }
        {   data data;
            durations[ 2 ] += json_parser< boost_json >::parse( data, json_string );
        }
#if HAVE_NLOHMANN_JSON
        {   data data;
            durations[ 3 ] += json_parser< nlohmann_json >::parse( data, json_string );
        }
#endif
#if HAVE_RAPIDJSON_JSON
        {   data data;
            durations[ 4 ] += json_parser< rapidjson_json >::parse( data, json_string );
        }
#endif
    }

    std::transform( durations.begin(), durations.end(), durations.begin(), [](auto d){ return d/100; } );

    std::cout << "\t\tptree\t\tQt\t\tboost\t\tnlohman\t\trapidjson\t|\tptree\t\tQt\t\tboost\t\tnlohman\t\trapidjson\n"
              << "read/write\t"
              << boost::format( "%8.3f" ) % durations[ 0 ]
              << "\t " << boost::format( "%8.3f" ) % durations[ 1 ]
              << "\t " << boost::format( "%8.3f" ) % durations[ 2 ]
              << "\t " << boost::format( "%8.3f" ) % durations[ 3 ]
              << "\t " << boost::format( "%8.3f" ) % durations[ 4 ]
              << "\t|"
              << "\t" << boost::format( "%8.3f" ) % (durations[0]/durations[reference])
              << "\t" << boost::format( "%8.3f" ) % (durations[1]/durations[reference])
              << "\t" << boost::format( "%8.3f" ) % (durations[2]/durations[reference])
              << "\t" << boost::format( "%8.3f" ) % (durations[3]/durations[reference])
              << "\t" << boost::format( "%8.3f" ) % (durations[4]/durations[reference]) << std::endl;

    std::fill( durations.begin(), durations.end(), 0 );

    // c++ -> json(string)
    for ( size_t i = 0; i < 100; ++i ) {
        durations[ 0 ] += json_parser< boost_ptree >::json_write( global_data );
        durations[ 1 ] += json_parser< qt5_json >::json_write( global_data );
        durations[ 2 ] += json_parser< boost_json >::json_write( global_data );
#if 0
        durations[ 3 ] += json_parser< nlohmann_json >::json_write( global_data );
        durations[ 4 ] += json_parser< rapidjson_json >::json_write( global_data );
#endif
    }

    std::transform( durations.begin(), durations.end(), durations.begin(), [](auto d){ return d/100; } );
    std::cout << "json_write\t"
              << boost::format( "%8.3f" ) % durations[ 0 ]
              << "\t " << boost::format( "%8.3f" ) % durations[ 1 ]
              << "\t " << boost::format( "%8.3f" ) % durations[ 2 ]
              << "\t " << boost::format( "%8.3f" ) % durations[ 3 ]
              << "\t " << boost::format( "%8.3f" ) % durations[ 4 ]
              << "\t|"
              << "\t" << boost::format( "%8.3f" ) % (durations[0]/durations[reference])
              << "\t" << boost::format( "%8.3f" ) % (durations[1]/durations[reference])
              << "\t" << boost::format( "%8.3f" ) % (durations[2]/durations[reference])
              << "\t" << boost::format( "%8.3f" ) % (durations[3]/durations[reference])
              << "\t" << boost::format( "%8.3f" ) % (durations[4]/durations[reference]) << std::endl;

    std::fill( durations.begin(), durations.end(), 0 );

    // json(string) -> c++ class
    for ( size_t i = 0; i < 100; ++i ) {
        { data data;
            durations[ 0 ] += json_parser< boost_ptree >::json_read( data, json_string );
        }
        { data data;
            durations[ 1 ] += json_parser< qt5_json >::json_read( data, json_string );
        }
        { data data;
            durations[ 2 ] += json_parser< boost_json >::json_read( data, json_string );
        }
#if HAVE_NLOHMANN_JSON
        { data data;
            durations[ 3 ] += json_parser< nlohmann_json >::json_read( data, json_string );
        }
#endif
#if HAVE_RAPIDJSON_JSON
        { data data;
            durations[ 4 ] += json_parser< rapidjson_json >::json_read( data, json_string );
        }
#endif
    }

    std::transform( durations.begin(), durations.end(), durations.begin(), [](auto d){ return d/100; } );

    std::cout << "json_read\t"
              << boost::format( "%8.3f" ) % durations[ 0 ]
              << "\t " << boost::format( "%8.3f" ) % durations[ 1 ]
              << "\t " << boost::format( "%8.3f" ) % durations[ 2 ]
              << "\t " << boost::format( "%8.3f" ) % durations[ 3 ]
              << "\t " << boost::format( "%8.3f" ) % durations[ 4 ]
              << "\t|"
              << "\t" << boost::format( "%8.3f" ) % (durations[0]/durations[reference])
              << "\t" << boost::format( "%8.3f" ) % (durations[1]/durations[reference])
              << "\t" << boost::format( "%8.3f" ) % (durations[2]/durations[reference])
              << "\t" << boost::format( "%8.3f" ) % (durations[3]/durations[reference])
              << "\t" << boost::format( "%8.3f" ) % (durations[4]/durations[reference]) << std::endl;

    {
        std::ofstream of( "ptree.json" );
        of << json_parser< boost_ptree >::stringify( global_data );
    }
    {
        std::ofstream of( "qt5.json" );
        of << json_parser< qt5_json >::stringify( global_data );
    }
    {
        std::ofstream of( "boost.json" );
        of << json_parser< boost_json >::stringify( global_data );
    }
#if HAVE_NLOHMANN_JSON
    {
        std::ofstream of( "nlohman.json" );
        of << json_parser< nlohmann_json >::stringify( global_data );
    }
#endif
#if HAVE_RAPIDJSON_JSON
    {
        std::ofstream of( "rapidjson.json" );
        of << json_parser< rapidjson_json >::stringify( global_data );
    }
#endif
}
