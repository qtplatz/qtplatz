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

#include "rapidjson_json.hpp"
#include "data.hpp"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>

#if 0
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#endif

rapidjson_json::rapidjson_json() : doc( std::make_unique< rapidjson::Document >() )
{
}

rapidjson_json::~rapidjson_json()
{
}

bool
rapidjson_json::parse( const std::string& json_string )
{
    doc->Parse( json_string.data() );
    return true;
}

std::string
rapidjson_json::stringify( const rapidjson::Document& d )
{
    rapidjson::StringBuffer buffer;
    buffer.Clear();
    rapidjson::Writer< rapidjson::StringBuffer > writer( buffer );
    d.Accept( writer );
    return buffer.GetString();
}

std::string
rapidjson_json::stringify( bool ) const
{
    return stringify( *doc );
}

bool
rapidjson_json::map( data& d )
{
    const auto& top = (*doc)["tick"];

    d.tick = std::stoul( top["tick"].GetString() );
    d.time = std::stoull( top["time"].GetString() );
    d.tick = std::stoul( top["time"].GetString() );

    {
        auto& values = top[ "hv" ][ "values" ];
        for ( auto it = values.Begin(); it != values.End(); ++it ) {
            tick::hv::value x;
            x.id   = std::stoul( (*it)[ "id" ].GetString() );
            x.name = (*it)[ "name" ].GetString();
            x.sn   = std::stoul( (*it)[ "sn" ].GetString() );
            std::string setpt = (*it)[ "set" ].GetString();
            x.set  = ( setpt == "n/a" ) ? 0 : std::stod( setpt );
            x.act  = std::stod( (*it)[ "act" ].GetString() );
            x.unit = (*it)[ "unit" ].GetString();
            d.values.emplace_back( x );
        }
    }
    
    d.alarm = top["alarms"]["alarm"]["text"].GetString();

    {
        auto& adc = top[ "adc" ];
        d.adc.tp = std::stoul( adc["tp"].GetString() );
        d.adc.nacc = std::stoul( adc["nacc"].GetString() );
        for ( auto it = adc["values"].Begin(); it != adc["values"].End(); ++it )
            d.adc.values.emplace_back( std::stod( it->GetString() ) );
    }

    return true;
}

std::string
rapidjson_json::make_json( const data& d )
{
    rapidjson::Document rj;
    rj.SetObject();
    
    rapidjson::Value top;
    top.SetObject();

    top.AddMember( "tick", d.tick, rj.GetAllocator() );
    top.AddMember( "time", d.time, rj.GetAllocator() );
    top.AddMember( "nsec", d.nsec, rj.GetAllocator() );

    {
        rapidjson::Value values;
        values.SetArray();
        for ( const auto& value: d.values ) {
            rapidjson::Value child;
            child.SetObject();
            child.AddMember( "id", value.id, rj.GetAllocator() );
            child.AddMember( "name", rapidjson::Value( value.name.c_str(), rj.GetAllocator() ).Move(), rj.GetAllocator() );
            child.AddMember( "sn", value.sn, rj.GetAllocator() );
            child.AddMember( "set", value.set, rj.GetAllocator() );
            child.AddMember( "act", value.act, rj.GetAllocator() );
            child.AddMember( "unit", rapidjson::Value( value.unit.c_str(), rj.GetAllocator() ).Move(), rj.GetAllocator() );
            values.PushBack( child, rj.GetAllocator() );
        }

        rapidjson::Value hv;
        hv.SetObject();
        hv.AddMember( "values", values, rj.GetAllocator() );
        top.AddMember( "hv", hv, rj.GetAllocator() );
    }

    rapidjson::Value alarms, alarm;
    alarms.SetObject();
    alarm.SetObject();
    alarm.AddMember( "text", rapidjson::Value( d.alarm.c_str(), rj.GetAllocator() ).Move(), rj.GetAllocator() );
    alarms.AddMember( "alarm", alarm, rj.GetAllocator() );
    top.AddMember( "alarms", alarms, rj.GetAllocator() );

    {
        rapidjson::Value adc;
        adc.SetObject();
        
        adc.AddMember( "tp", d.adc.tp, rj.GetAllocator() );
        adc.AddMember( "nacc", d.adc.nacc, rj.GetAllocator() );
        
        rapidjson::Value values( rapidjson::kArrayType );

        for ( auto& value:  d.adc.values ) {
            values.PushBack( value, rj.GetAllocator() );
        }

        adc.AddMember( "values", values, rj.GetAllocator() );

        top.AddMember( "adc", adc, rj.GetAllocator() );
    }

    rj.AddMember( "tick", top, rj.GetAllocator() );

#if 0
    std::istringstream in( stringify( rj ) );
    boost::property_tree::ptree pt;
    boost::property_tree::read_json( in, pt );
    boost::property_tree::write_json( std::cerr, pt );
    // std::cerr << stringify( rj ) << std::endl;
#endif

    return stringify( rj );
}


