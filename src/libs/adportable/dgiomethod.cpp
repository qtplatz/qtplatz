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

#include "dgioconfig.hpp"
#include "dgiomethod.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <iostream>

using namespace adportable::dg;

method::method() : id_( 0 )
{
}

method::method( const method& t ) : id_( t.id_ )
                                  , title_( t.title_ )
                                  , prepare_( t.prepare_ )
                                  , table_( t.table_ )
{
}

void
method::setConfig( const configuration& c )
{
    for ( auto& io: c ) {
        if ( io.mode_ == OUT && io.enable_ ) {
            tAction a = io.initState_ == High ? ioHigh : ioLow;
            prepare_.emplace_back( iEvent{ io.id_, a } );
        }
    }
}


bool
method::read_json( std::istream& json, method& m )
{
    boost::property_tree::ptree pt;
    
    try {
        boost::property_tree::read_json( json, pt );

        if ( auto value = pt.get_optional< int >( "method.id" ) )
            m.id() = value.get();
        else
            return false;

        if ( auto value = pt.get_optional< std::string >( "method.title" ) )
            m.title() = value.get();

        if ( auto child = pt.get_child_optional( "method.prepare" ) ) {

            m.prepare().clear();

            for ( const auto& item: child.get() ) {
                iEvent e;
                if ( boost::optional< int > value = item.second.get_optional<int>( "id" ) )
                    e.pid_   = value.get();
                if ( boost::optional< int > value = item.second.get_optional<int>( "action" ) )
                    e.action_ = tAction( value.get() );
                
                m.prepare().emplace_back( e );
            }
        } else
            return false;

        if ( auto child = pt.get_child_optional( "method.table" ) ) {

            m.table().clear();
            
            for ( const auto& item: child.get() ) {
                tEvent e;
                if ( boost::optional< int > value = item.second.get_optional<int>( "pid" ) )
                    e.pid_   = value.get();
                else
                    return false;
                if ( boost::optional< double > value = item.second.get_optional<double>( "elapsed_time" ) )
                    e.elapsed_time_ = value.get();
                if ( boost::optional< int > value = item.second.get_optional<int>( "action" ) )
                    e.action_ = tAction( value.get() );
                
                m.table().emplace_back( e );
            }
        } else
            return false;

        return true;

    } catch ( std::exception& e ) {
        std::cout << boost::diagnostic_information( e ) << std::endl;
    }
    return false;
}

bool
method::write_json( std::ostream& json, const method& m )
{
    boost::property_tree::ptree pt, pv, tv;

    pt.put( "method.id", m.id() );
    pt.put( "method.title", m.title() );

    for ( const auto& iv: m.prepare() ) {

        boost::property_tree::ptree xitem;

        xitem.put( "pid",          iv.pid_ );
        xitem.put( "action",       iv.action_ );

        pv.push_back( std::make_pair( "", xitem ) );
    }
    
    pt.add_child( "method.prepare", pv );
    
    for ( const auto& ev: m.table() ) {

        boost::property_tree::ptree xitem;
        
        xitem.put( "elapsed_time", ev.elapsed_time_ );
        xitem.put( "action",       ev.action_ );
        xitem.put( "pid",          ev.pid_ );
        
        tv.push_back( std::make_pair( "", xitem ) );
    }
    
    pt.add_child( "method.table", tv );

    boost::property_tree::write_json( json, pt );
    
    return true;
}

