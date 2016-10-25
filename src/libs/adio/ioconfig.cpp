// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "ioconfig.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <iostream>

static void
print( const boost::property_tree::ptree& pt )
{
    using boost::property_tree::ptree;

    ptree::const_iterator end = pt.end();
    for (ptree::const_iterator it = pt.begin(); it != end; ++it) {
        std::cout << it->first << ": " << it->second.get_value<std::string>() << std::endl;
        print(it->second);
    }    
}

using namespace adio::io;

ioConfig::ioConfig() : enable_( false )
                     , id_( 0 )
                     , mode_( OUT )
                     , trigConfig_( Edge | Negative )
                     , initState_( Low )
{
}

ioConfig::ioConfig( const ioConfig& t ) : enable_( t.enable_ )
                                        , id_( t.id_ )
                                        , mode_( t.mode_ )
                                        , trigConfig_( t.trigConfig_ )
                                        , initState_( t.initState_ )
                                        , name_( t.name_ )
                                        , note_( t.note_ )
{
}

ioConfig::ioConfig( bool enable
                    , uint32_t id
                    , ioMode mode
                    
                    , uint32_t trig
                    , ioState initState
                    , const std::string& name
                    , const std::string& note ) : enable_( enable )
                                                , id_( id )
                                                , mode_( mode )
                                                , trigConfig_( trig )
                                                , initState_( initState )
                                                , name_( name )
                                                , note_( note )
{
}

////////////////
configuration::configuration()
{
    uint32_t id(1);
    
    config_.emplace_back( true, id++, IN, Edge | Negative, High, "START-IN" );   // DE0 SW-0
    config_.emplace_back( true, id++, IN, Edge | Negative, High, "INJECT-IN" );  // DE0 SW-1
    config_.emplace_back( true, id++, IN,  Level, Low, "IN"  );                  // DE0 DIPSW(0)
    config_.emplace_back( true, id++, IN,  Level, Low, "IN"  );                  // DE0 DIPSW(1)
    config_.emplace_back( true, id++, IN,  Level, Low, "IN"  );                  // DE0 DIPSW(2)
    config_.emplace_back( true, id++, IN,  Level, Low, "IN"  );                  // DE0 DIPSW(3)
    config_.emplace_back( true, id++, OUT, Level, Low, "OUT" );                  // DE0 GPIO(n)
    config_.emplace_back( true, id++, OUT, Level, Low, "OUT" );                  // DE0 GPIO(n)
    config_.emplace_back( true, id++, OUT, Level, Low, "OUT" );                  // DE0 GPIO(n)
    config_.emplace_back( true, id++, OUT, Level, Low, "OUT" );                  // DE0 GPIO(n)
}

configuration::configuration( const configuration& t ) : config_( t.config_ )
{
}

void
configuration::clear()
{
    config_.clear();
}

bool
configuration::read_json( const boost::property_tree::ptree& item, ioConfig& c )
{
    if ( boost::optional< int > value = item.get_optional<int>( "id" ) )
        c.id_   = value.get();
    else
        return false;

    if ( boost::optional< bool > value = item.get_optional<bool>( "enable" ) )
        c.enable_ = value.get();
    
    if ( boost::optional< int > value = item.get_optional<int>( "mode" ) )
        c.mode_  = ioMode( value.get() );

    if ( boost::optional< int > value = item.get_optional<int>( "trigConfig" ) )
        c.trigConfig_ = value.get();

    if ( boost::optional< int > value = item.get_optional<int>( "initState" ) )
        c.initState_ = ioState( value.get() );

    if ( boost::optional< std::string > value = item.get_optional<std::string>( "name" ) )
        c.name_ = value.get();

    if ( boost::optional< std::string > value = item.get_optional<std::string>( "note" ) )
        c.note_ = value.get();

    return true;
}

bool
configuration::read_json( std::istream& json, ioConfig& io )
{
    boost::property_tree::ptree pt;
    
    try {

        boost::property_tree::read_json( json, pt );
        return read_json( pt, io );
        
    } catch ( std::exception& e ) {

        std::cerr << boost::diagnostic_information( e );
        return false;

    }
}

bool
configuration::read_json( std::istream& json, configuration& config )
{
    config.clear();
    
    boost::property_tree::ptree pt;
    
    try {
        boost::property_tree::read_json( json, pt );

        if ( auto child = pt.get_child_optional( "ioConfig" ) ) {

            for ( const auto& item: child.get() ) { // pt.get_child( "ioConfig" ) ) {

                ioConfig c;
                read_json( item.second, c );
                config.config().emplace_back( c );
            }

            return true;

        } else if ( auto id = pt.get_child_optional( "id" ) ) {
            ioConfig c;            
            if ( read_json( pt, c ) )
                config.config().emplace_back( c );
            return true;
        }

    } catch ( std::exception& e ) {
        std::cerr << boost::diagnostic_information( e );
        return false;
    }
    return false;
}

bool
configuration::write_json( std::ostream& json, const configuration& config )
{
    boost::property_tree::ptree pv;

    for ( const auto& item: config ) {

        boost::property_tree::ptree xitem;

        xitem.put( "enable",     item.enable_ );
        xitem.put( "id",         item.id_ );
        xitem.put( "mode",       item.mode_ );
        xitem.put( "trigConfig", item.trigConfig_ );
        xitem.put( "initState",  item.initState_ );
        xitem.put( "name",       item.name_ );
        xitem.put( "note",       item.note_ );
        
        pv.push_back( std::make_pair( "", xitem ) );
    }

    boost::property_tree::ptree pt;
    
    pt.add_child( "ioConfig", pv );

    boost::property_tree::write_json( json, pt );
    
    return true;
}

////////////////////////////////////////

