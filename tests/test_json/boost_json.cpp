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
#include "data.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/foreach.hpp>
#include <iostream>

boost_json::boost_json() : ptree( std::make_unique< boost::property_tree::ptree >() )
{
}

boost_json::~boost_json()
{
}

bool
boost_json::parse( const std::string& json_string )
{
    boost::iostreams::basic_array_source< char > device( json_string.data(), json_string.size() );
    boost::iostreams::stream< boost::iostreams::basic_array_source< char > > in( device );
    try {
        boost::property_tree::read_json( in, *ptree );
    } catch ( std::exception& ex ) {
        std::cerr << ex.what() << std::endl;
        return false;
    }
    return true;
}

std::string
boost_json::stringify( bool pritty ) const
{
    std::ostringstream o;
    boost::property_tree::write_json( o, *ptree, pritty );
    if ( !pritty )
        return o.str().substr( 0, o.str().find_first_of( "\r\n" ) );
    return o.str();
}

std::string
boost_json::stringify( const boost::property_tree::ptree& pt, bool pritty )
{
    std::ostringstream o;
    boost::property_tree::write_json( o, pt, pritty );
    if ( !pritty )
        return o.str().substr( 0, o.str().find_first_of( "\r\n" ) );
    return o.str();
}

bool
boost_json::map( data& d )
{
    if ( auto tick = ptree->get_child_optional( "tick" ) ) {
        if ( auto value = tick->get_optional< uint32_t >( "tick" ) )
            d.tick = value.get();
        if ( auto value = tick->get_optional< uint64_t >( "time" ) )
            d.time = value.get();
        if ( auto value = tick->get_optional< uint32_t >( "nsec" ) )
            d.nsec = value.get();
        if ( auto hv = tick->get_child_optional( "hv" ) ) {
            BOOST_FOREACH( const boost::property_tree::ptree::value_type& v, hv->get_child( "values" ) ) {
                tick::hv::value x;
                if ( auto value = v.second.get_optional< uint32_t >( "id" ) )
                    x.id = value.get();
                if ( auto value = v.second.get_optional< std::string >( "name" ) )
                    x.name = value.get();
                if ( auto value = v.second.get_optional< uint32_t >( "sn" ) )
                    x.sn = value.get();
                if ( auto value = v.second.get_optional< double >( "set" ) )
                    x.set = value.get();
                if ( auto value = v.second.get_optional< double >( "act" ) )
                    x.act = value.get();                
                if ( auto value = v.second.get_optional< std::string >( "unit" ) )
                    x.unit = value.get();
                d.values.emplace_back( x );
            }            
        }
        if ( auto alarm = tick->get_child_optional( "alarms.alarm" ) ) {
            if ( auto value = alarm->get_optional< std::string >( "text" ) )
                d.alarm = value.get();
        }
        
        if ( auto adc = tick->get_child_optional( "adc" ) ) {
            if ( auto value = adc->get_optional< uint64_t >( "tp" ) )
                d.adc.tp = value.get();
            if ( auto value = adc->get_optional< uint32_t >( "nacc" ) )
                d.adc.nacc = value.get();
            BOOST_FOREACH( const boost::property_tree::ptree::value_type& v, adc->get_child( "values" ) ) {
                d.adc.values.emplace_back( v.second.get_value< double >() );
            }
        }        
    }
}

std::string
boost_json::make_json( const data& d )
{
    boost::property_tree::ptree pt, hv, alarms, adc;
    pt.put( "tick.tick", d.tick );
    pt.put( "tick.time", d.time );
    pt.put( "tick.nsec", d.nsec );

    for ( const auto& value: d.values ) {
        boost::property_tree::ptree child;
        child.put( "id", value.id );
        child.put( "name", value.name );
        child.put( "sn", value.sn );
        child.put( "set", value.set );
        child.put( "act", value.act );
        child.put( "unit", value.unit );
        hv.push_back( std::make_pair( "", child ) );
    }

    alarms.put( "alarm.text", d.alarm );

    adc.put( "tp", d.adc.tp );
    adc.put( "nacc", d.adc.nacc );

    boost::property_tree::ptree child;
    for ( const auto& value: d.adc.values ) {
        boost::property_tree::ptree item;
        item.put( "", value );
        child.push_back( std::make_pair( "", item ) );
    }
    adc.add_child( "values", child ); // adc

    pt.add_child( "tick.hv", hv );
    pt.add_child( "tick.alarms", alarms );
    pt.add_child( "tick.adc", adc );

    return stringify( pt );
}
