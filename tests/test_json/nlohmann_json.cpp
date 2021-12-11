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

#include "nlohmann_json.hpp"
#include "data.hpp"
#include <iostream>
#include <sstream>

nlohmann_json::nlohmann_json()
{
}

nlohmann_json::~nlohmann_json()
{
}

bool
nlohmann_json::parse( const std::string& json_string )
{
    try {
        json = nlohmann::json::parse( json_string );
    } catch ( std::exception& ex ) {
        std::cerr << __FILE__ << " " << __LINE__ << " " << __FUNCTION__ << " " << ex.what() << std::endl;
        std::cerr << json_string << std::endl;
    }
    return true;
}

std::string
nlohmann_json::stringify( bool ) const
{
    std::ostringstream o;
    o << json;
    return o.str();
}

bool
nlohmann_json::map( data& d )
{
    const auto& top = json.at( "tick" );

    try {
        d.tick = top.at( "tick" ).get< decltype( d.tick ) >();
        d.time = top.at( "time" ).get< decltype( d.time ) >();
        d.nsec = top.at( "nsec" ).get< decltype( d.nsec ) >();
    } catch ( std::exception& ex ) {
        std::cerr << __FILE__ << ":" << __LINE__ << " exception: " << ex.what() << std::endl;
        return false;
    }

    try {
        {
            auto& values = top[ "hv" ][ "values" ];
            for ( const auto& value: values ) {
                tick::hv::value x;
                x.id   = value.at( "id" ).get< decltype( x.id ) >();
                x.name = value.at( "name" ).get< std::string >();
                x.sn   = value.at( "sn" ).get< decltype( x.sn ) >();
                x.set = value.at( "set" ).get< decltype( x.set ) >();
                x.act  = value.at( "act" ).get< decltype( x.act ) >();
                x.unit = value.at( "unit" ).get< std::string >();
                d.values.emplace_back( x );
            }
        }
    } catch ( std::exception& ex ) {
        std::cerr << __FILE__ << ":" << __LINE__ << " exception: " << ex.what() << std::endl;
        return false;
    }
#if 0
    try {
        auto alarm = top["alarms"]["alarm"];
        std::cerr << alarm.dump() << std::endl;
        d.alarm = alarm.at( "text" ).get< std::string >();
    } catch ( std::exception& ex ) {
        std::cerr << __FILE__ << ":" << __LINE__ << " exception: " << ex.what() << std::endl;
    }
#endif
    try {
        auto& adc = top.at( "adc" );
        d.adc.tp = adc["tp"].get< decltype( d.adc.tp ) >();
        d.adc.nacc = adc["nacc"].get< decltype( d.adc.nacc ) >();
        for ( auto& value: adc["values"] )
            d.adc.values.emplace_back( value.get< double >() );
    } catch ( std::exception& ex ) {
        std::cerr << __FILE__ << ":" << __LINE__ << " exception: " << ex.what() << std::endl;
        return false;
    }
    return true;
}

std::string
nlohmann_json::make_json( const data& d )
{
    using json = nlohmann::json;

    json j =
        { { "tick"
            , { { "tick", d.tick }
                , {"time", d.time }
                , {"nsec", d.nsec }
                , {"hv"
                   , { {"values", json::array({}) } }
                    }
                , { "alarms"
                    , { { "alarm", { { "text", d.alarm } } } }
                    }
                , { "adc"
                    , { { "tp", d.adc.tp }
                        , { "nacc", d.adc.nacc }
                        , { "values", d.adc.values }
                        }
                    }
                }
            }
        };

    json j_values;

    for ( const auto& value: d.values ) {
        json j_value =
        {
            { "id", value.id }
            , { "name", value.name }
            , { "sn", value.sn }
            , { "set", value.set }
            , { "act", value.act }
            , { "unit", value.unit }
        };
        j_values.push_back( j_value );
    }

    j["tick"]["hv"]["values"] = j_values;

    std::ostringstream o;
    o << j;
    return o.str();
}
