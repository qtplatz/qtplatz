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

#include "boost_json.hpp"
#include "qt5_json.hpp"
#include "nlohmann_json.hpp"
#include "rapidjson_json.hpp"
#include "data.hpp"
#include <boost/format.hpp>
#include <chrono>
#include <iostream>
#include <fstream>

void
load_data( std::string& data )
{
    std::ifstream inf( DATAFILE );
    data = std::string((std::istreambuf_iterator<char>(inf)),  (std::istreambuf_iterator<char>()));    
}

int
main()
{
    std::string json_string;
    load_data( json_string );

    std::ofstream null( "/dev/null" );
    
    std::vector< double > durations;
    {  //  boost --
        typedef boost_json json_parser;

        json_parser parser;
        std::string json_readback, json_stringify;
        data data;

        auto tp0 = std::chrono::steady_clock::now();
        if ( parser.parse( json_string ) ) {
            json_stringify = parser.stringify();
            parser.map( data );
            json_readback = parser.make_json( data );
        }
        auto tp = std::chrono::steady_clock::now();
        durations.emplace_back( double( std::chrono::nanoseconds( tp - tp0 ).count() ) / 1e3 );
        null << "stringify: " << json_stringify << std::endl;
        null << "readback: " << json_readback << std::endl;
    }
    
    {  // Qt
        typedef qt5_json json_parser;
        json_parser parser;
        std::string json_readback, json_stringify;
        data data;
        
        auto tp0 = std::chrono::steady_clock::now();
        if ( parser.parse( json_string ) )  {
            json_stringify = parser.stringify();
            parser.map( data );
            json_readback = parser.make_json( data );
        }
        auto tp = std::chrono::steady_clock::now();
        durations.emplace_back( double( std::chrono::nanoseconds( tp - tp0 ).count() ) / 1e3 );
        null << "stringify: " << json_stringify << std::endl;
        null << "readback: " << json_readback << std::endl;
    }

    { // nlohmann
        typedef nlohmann_json json_parser;
        
        json_parser parser;
        std::string json_readback, json_stringify;
        data data;
        auto tp0 = std::chrono::steady_clock::now();

        if ( parser.parse( json_string ) )  {
            json_stringify = parser.stringify();
            parser.map( data );
            json_readback = parser.make_json( data );
        }

        auto tp = std::chrono::steady_clock::now();
        durations.emplace_back( double( std::chrono::nanoseconds( tp - tp0 ).count() ) / 1000.0 );
        null << "stringify: " << json_stringify << std::endl;
        null << "readback: " << json_readback << std::endl;
        // std::cout << boost_json::make_json( data );
    }
    
    { // RapidJSON
        typedef rapidjson_json json_parser;

        json_parser parser;
        std::string json_readback, json_stringify;
        data data;
        
        auto tp0 = std::chrono::steady_clock::now();

        if ( parser.parse( json_string ) ) {
            json_stringify = parser.stringify();
        }

        auto tp = std::chrono::steady_clock::now();
        durations.emplace_back( double( std::chrono::nanoseconds( tp - tp0 ).count() ) / 1000.0 );
        null << "stringify: " << json_stringify << std::endl;
        null << "readback: " << json_readback << std::endl;                
    }

    std::cout << "Boost,Qt,nlohmann,Rapid\t"
              << boost::format( "%8.3f" ) % durations[ 0 ]
              << "\t " << boost::format( "%8.3f" ) % durations[ 1 ]
              << "\t " << boost::format( "%8.3f" ) % durations[ 2 ]
              << "\t " << boost::format( "%8.3f" ) % durations[ 3 ]
              << "\t|"
              << "\t" << boost::format( "%8.3f" ) % (durations[0]/durations[3])
              << "\t" << boost::format( "%8.3f" ) % (durations[1]/durations[3])
              << "\t" << boost::format( "%8.3f" ) % (durations[2]/durations[3])
              << "\t" << boost::format( "%8.3f" ) % (durations[3]/durations[3]) << std::endl;
}
