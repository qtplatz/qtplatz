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

namespace {
    template<class T>
    void extract( const rapidjson::Value& obj, T& t );

    template<> void extract( const rapidjson::Value& obj, double& t )  {
        t = obj.GetDouble();
    }
    template<> void extract( const rapidjson::Value& obj, int64_t& t )  {
        t = obj.GetInt64();
    }
    template<> void extract( const rapidjson::Value& obj, uint64_t& t )  {
        t = obj.GetUint64();
    }
    template<> void extract( const rapidjson::Value& obj, int& t )  {
        t = obj.GetInt();
    }
    template<> void extract( const rapidjson::Value& obj, unsigned int& t )  {
        t = obj.GetUint();
    }
    template<> void extract( const rapidjson::Value& obj, bool& t )  {
        t = obj.GetBool();
    }
    template<> void extract( const rapidjson::Value& obj, std::string& t )  {
        t = obj.GetString();
    }
}

rapidjson_json::rapidjson_json() : doc( std::make_unique< rapidjson::Document >() )
{
}

rapidjson_json::~rapidjson_json()
{
}

bool
rapidjson_json::parse( const std::string& json_string )
{
    try {
        doc->Parse( json_string.data() );
    } catch ( std::exception &ex ) {
        std::cout << "line: " << __LINE__ << " exception: " << ex.what() << std::endl;
        return false;
    }
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
    try {
        extract( top[ "tick" ], d.tick );
        extract( top[ "time" ], d.time );
        extract( top[ "nsec" ], d.nsec );

        auto& values = top[ "hv" ][ "values" ];
        for ( auto it = values.Begin(); it != values.End(); ++it ) {
            tick::hv::value x;
            extract( (*it)[ "id" ], x.id );
            extract( (*it)[ "name" ], x.name );
            extract( (*it)[ "sn" ], x.sn );
            extract( (*it)[ "set" ], x.set );
            extract( (*it)[ "act" ], x.act );
            extract( (*it)[ "unit" ], x.unit );
            d.values.emplace_back( x );
        }

        d.alarm = top[ "hv" ]["alarms"]["alarm"]["text"].GetString();

        auto& adc = top[ "adc" ];
        extract( adc[ "tp" ], d.adc.tp );
        extract( adc[ "nacc" ], d.adc.nacc );
        for ( auto it = adc["values"].Begin(); it != adc["values"].End(); ++it )
            d.adc.values.emplace_back( it->GetDouble() );
    } catch ( std::exception& ex ) {
        std::cerr << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " exception: " << ex.what() << std::endl;
        return false;
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

    return stringify( rj );
}
