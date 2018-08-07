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
    auto top = json.at( "tick" );

    d.tick = std::stol( top[ "tick" ].get< std::string >() );
    d.time = std::stoull( top[ "time" ].get< std::string >() );
    d.nsec = std::stol( top[ "tick" ].get< std::string >() );

    {
        auto& values = top[ "hv" ][ "values" ];
        for ( const auto& value: values ) {
            tick::hv::value x;
            x.id   = std::stol( value.at( "id" ).get< std::string >() );
            x.name = value.at( "name" ).get< std::string >();
            x.sn   = std::stoul( value.at( "sn" ).get< std::string >() );
            auto set = value.at( "set" ).get< std::string >();
            x.set = ( set == "n/a" ) ? 0 : std::stod( set );
            x.act  = std::stod( value.at( "act" ).get< std::string >() );
            x.unit = value.at( "unit" ).get< std::string >();
            d.values.emplace_back( x );
        }
    }
    
    d.alarm = top["alarms"]["alarm"]["text"].get< std::string >();

    {
        auto& adc = top.at( "adc" );
        d.adc.tp = std::stoull( adc["tp"].get< std::string >() );
        d.adc.nacc = std::stoul( adc["nacc"].get< std::string >() );
        for ( auto& value: adc["values"] )
            d.adc.values.emplace_back( std::stod( value.get< std::string >() ) );
    }
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

