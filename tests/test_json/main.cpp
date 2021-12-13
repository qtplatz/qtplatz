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
#include "boost_json.hpp"
#if HAVE_Qt5_JSON
#include "qt5_json.hpp"
#endif
#if HAVE_NLOHMANN_JSON
#include "nlohmann_json.hpp"
#endif
#if HAVE_RAPIDJSON_JSON
#include "rapidjson_json.hpp"
#endif
#include "data.hpp"
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

constexpr const int reference = 0;
constexpr const size_t replicates = 250;
data global_data;

bool
is_equal( double a, double b ) {
    return std::abs(a - b) <= ( (std::abs(a) < std::abs(b) ? std::abs(b)
                                 : std::abs(a)) * std::numeric_limits< double >::epsilon() );
}

bool
is_equal( const std::vector< double>& v1, const std::vector< double >& v2 )
{
    if ( v1.size() == v2.size() ) {
        for ( size_t i = 0; i < v1.size(); ++i ) {
            if ( ! is_equal( v1[ i ], v2[ i ] ) )
                return false;
        }
        return true;
    }
    return false;
}


std::string
load_data()
{
    std::ifstream inf( DATAFILE ); // this was created by pteee (not comformed json)

    auto ptree_str = std::string((std::istreambuf_iterator<char>(inf)),  (std::istreambuf_iterator<char>()));
    boost_ptree pt;
    pt.parse( ptree_str );

    try {
        pt.map( global_data );
        auto json = boost_json().make_json( global_data ); // make conformed json string

        std::ofstream o("global.json");
        o << json;

        return json;

    } catch ( std::exception& ex ) {
        std::cerr << __FILE__ << ":" << __LINE__ << " exception: " << ex.what() << std::endl;
    }
    return {};
}

template< typename T >
struct json_parser {
    double static parse( const std::string& json_string ) {
        T parser;
        auto tp0 = std::chrono::steady_clock::now();
        if ( parser.parse( json_string ) ) {
            double dur = double( std::chrono::nanoseconds( std::chrono::steady_clock::now() - tp0 ).count() ) / 1e3;
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
        if ( parser.parse( json_string ) ) {
            auto tp0 = std::chrono::steady_clock::now();
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

template< typename... Types> struct parser_list {};

struct null_parser {};

using parsers = parser_list<
    boost_json
    , boost_ptree
#if HAVE_Qt5_JSON
    , qt5_json
#endif
#if HAVE_NLOHMANN_JSON
    , nlohmann_json
#endif
#if HAVE_RAPIDJSON_JSON
    , rapidjson_json
#endif
    , null_parser
    >;

enum parser_id {
    id_boost_json
    , id_boost_ptree
#if HAVE_Qt5_JSON
    , id_qt5_json
#endif
#if HAVE_NLOHMANN_JSON
    , id_nlohmann_json
#endif
#if HAVE_RAPIDJSON_JSON
    , id_rapidjson_json
#endif
    , id_null_parser
};

const std::vector< std::string > parser_names = {
    "boost_json"
    , "boost_ptree"
#if HAVE_Qt5_JSON
    , "qt5_json"
#endif
#if HAVE_NLOHMANN_JSON
    , "nlohmann"
#endif
#if HAVE_RAPIDJSON_JSON
    , "rapidjson"
#endif
};

template< typename last_t> struct parser_list< last_t > {
    std::string print( std::ostringstream&& o, size_t ) const { return o.str(); }
    std::vector< double > parser( const std::string&, std::vector< double >&& d ) const { return d; }
    std::vector< double > json_write( const data& data, std::vector< double >&& d ) const { return d; }
    std::vector< std::pair< double, data > > json_read( const std::string& json_string
                                                        , std::vector< std::pair< double, data > >&& d ) const { return d; }
    void json_read_verify( const std::string& json_string, const data& reference, size_t ) const {}
    void json_serialize( const data& data, size_t idx = 0 ) const {}
};

template< typename first_t, typename... args> struct parser_list< first_t, args ...> {
    std::string print( std::ostringstream&& o = std::ostringstream(), size_t idx = 0 ) const {
        o << parser_names[ idx ] << "\t";
        return parser_list< args ... >().print( std::move( o ), idx + 1 );
    }

    std::vector< double >
    parser( const std::string& json_string, std::vector< double >&& d ) const {
        d.emplace_back( json_parser< first_t >::parse( json_string ) );
        return parser_list< args ... >().parser( json_string, std::move( d  ));
    }

    std::vector< double >
    json_write( const data& data, std::vector< double >&& d ) const {
        d.emplace_back( json_parser< first_t >::json_write( data ) );
        return parser_list< args ... >().json_write( data, std::move( d  ));
    }

    std::vector< std::pair< double, data > >
    json_read( const std::string& json_string, std::vector< std::pair< double, data > >&& d ) const {
        data t;
        d.emplace_back( json_parser< first_t >::json_read( t, json_string ), std::move( t ) );
        return parser_list< args ... >().json_read( json_string, std::move( d  ) );
    }

    void
    json_read_verify( const std::string& json_string, const data& reference, size_t idx = 0 ) const {
        data t;
        json_parser< first_t >::json_read( t, json_string );
        if ( !( t == reference ) ) {
            auto where = t.compare( reference );
            std::cout << "bad data in " << parser_names[ idx ] << " at " << where << std::endl;
        }
        return parser_list< args ... >().json_read_verify( json_string, reference, idx + 1 );
    }

    void
    json_serialize( const data& data, size_t idx = 0 ) const {
        std::ofstream of( parser_names[ idx ] + ".json" );
        of << json_parser< first_t >::stringify( data );

        parser_list< args ... >().json_serialize( data, idx + 1 );
    }
};

void
report( const std::string& title, const std::array< double, id_null_parser >& durations, bool heading = false )
{
    // head line
    if ( heading ) {
        parsers parsers;
        std::cout << "\t\t" << parsers.print() << "|\t" <<  parsers.print() << std::endl;
    }
    //
    std::cout << title << "\t";
    for ( const auto& dur: durations )
        std::cout << boost::format( "%8.3f" ) % dur << "\t";
    std::cout << "|\t";
    for ( const auto& dur: durations )
        std::cout << boost::format( "%8.3f" ) % (dur/durations[reference]) << "\t";
    std::cout << std::endl;
}

int
main()
{
    const std::string json_string = load_data();
    std::array< double, id_null_parser > durations = { 0 };
    parsers parsers;

    try {
        for ( size_t i = 0; i < replicates; ++i ) {
            auto dur = parsers.parser( json_string, std::vector<double>{} );
            transform( dur.begin(), dur.end(), durations.begin(), durations.begin()
                       , [](const double& a, const double& b){ return a+b; });

        }
    } catch ( std::exception& ex ) {
        std::cerr << __FILE__ << ":" << __LINE__ << " exception: " << ex.what();
    }
    std::transform( durations.begin(), durations.end(), durations.begin(), [](auto d){ return d/replicates; } );
    report( "json parse", durations, true );

    std::fill( durations.begin(), durations.end(), 0 );
    try {
        for ( size_t i = 0; i < replicates; ++i ) {
            auto dur = parsers.json_write( global_data, std::vector<double>{} );
            transform( dur.begin(), dur.end(), durations.begin(), durations.begin(), [](const double& a, const double& b){ return a+b; });
        }
    } catch ( std::exception& ex ) {
        std::cerr << __FILE__ << ":" << __LINE__ << " exception: " << ex.what();
    }
    std::transform( durations.begin(), durations.end(), durations.begin(), [](auto d){ return d/replicates; } );
    report( "json write", durations );

    try {
        std::fill( durations.begin(), durations.end(), 0 );
        // json(string) -> c++ class
        for ( size_t i = 0; i < replicates; ++i ) {
            auto r = parsers.json_read( json_string, std::vector< std::pair< double, data > >{} );
            transform( r.begin(), r.end(), durations.begin(), durations.begin(), [](const auto& a, const auto& b){ return a.first + b; });
        }
    } catch ( std::exception& ex ) {
        std::cerr << __FILE__ << ":" << __LINE__ << " exception: " << ex.what() << std::endl;
    }
    std::transform( durations.begin(), durations.end(), durations.begin(), [](auto d){ return d/replicates; } );
    report( "json read", durations );

    try {
        parsers.json_serialize( global_data );
    } catch ( std::exception& ex ) {
        std::cerr << __FILE__ << ":" << __LINE__ << " exception: " << ex.what();
    }

    ////////////
    // data validation
    try {
        parsers.json_read_verify( json_string, global_data );
    } catch ( std::exception& ex ) {
        std::cerr << __FILE__ << ":" << __LINE__ << " exception: " << ex.what();
    }

    std::cout << "\n\tjson read test\n";

    typedef boost_json reference_parser;

    for ( const auto& name: parser_names ) {

        std::ifstream inf( name + ".json" );
        auto json_str = std::string((std::istreambuf_iterator<char>(inf)),  (std::istreambuf_iterator<char>()));
        data dt;
        reference_parser parser;
        try {
            parser.parse( json_str );
            parser.map( dt );
            if ( !( dt == global_data ) ) {
                auto where = dt.compare( global_data );
                std::cout << "\tbad data in " << name << " at " << where << std::endl;
            } else {
                std::cout << "\t" << name << ".json\t" << "ok";
            }
        } catch ( std::exception& ex ) {
            std::cerr << boost::diagnostic_information( ex ) << std::endl;
        }
    }
    std::cout << std::endl;
}
