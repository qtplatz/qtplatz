// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "debug.hpp"
#include <adportable/string.hpp>
#include "configloader.hpp"
#include <adportable/configuration.hpp>
#include <xmlparser/pugiwrapper.hpp>
#include <pugixml.hpp>
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>

using namespace adportable;

struct ConfigLoaderImpl {
    // pugi
    static bool populate( Configuration&, const pugi::xml_node& );
    static bool load( Configuration&, const pugi::xml_node& );
    static bool resolve_module( Configuration&, const pugi::xml_node& );

};

ConfigLoader::ConfigLoader(void)
{
}

ConfigLoader::~ConfigLoader(void)
{
}

///////////////////////// using pugi interface ///////////

// static
bool
ConfigLoader::loadConfigFile( adportable::Configuration& config, const std::wstring& file, const std::wstring& query )
{
    pugi::xml_document dom;
    pugi::xml_parse_result result = dom.load_file( pugi::as_utf8( file ).c_str() );
    if ( ! result ) {
        adportable::debug dbg( __FILE__, __LINE__ );
        dbg << "adportable::ConfigLoader::loadConfigFile(\"" << file << "\")" << result.description();
        return false;
    }

    pugi::xpath_node_set list = dom.select_nodes( pugi::as_utf8( query ).c_str() );
    if ( list.size() == 0 )
        return false;

    if ( list.size() == 1 ) {
        if ( ConfigLoaderImpl::load( config, list[0].node() ) )
            ConfigLoaderImpl::populate( config, list[0].node() );
        return true;
    }

    for ( size_t i = 0; i < list.size(); ++i ) {
        Configuration& child = config.append( Configuration() );
        if ( ConfigLoaderImpl::load( child, list[i].node() ) )
            ConfigLoaderImpl::populate( child, list[i].node() );
    }
    return true;
}

// static
bool
ConfigLoader::loadConfigXML( adportable::Configuration& config, const std::wstring& xml, const std::wstring& query )
{
    pugi::xml_document dom;
    pugi::xml_parse_result result;
    if ( ! ( result = dom.load_string( pugi::as_utf8( xml ).c_str() ) ) )
        return false;

    pugi::xpath_node_set list = dom.select_nodes( pugi::as_utf8( query ).c_str() );
    if ( list.size() == 0 )
        return false;

    if ( list.size() == 1 ) {
        if ( ConfigLoaderImpl::load( config, list[0].node() ) )
            ConfigLoaderImpl::populate( config, list[0].node() );
    } else
        return false;
    return true;
}

//
bool
ConfigLoader::load( Configuration& config, const pugi::xml_node& node )
{
    return ConfigLoaderImpl::load( config, node );
}

bool
ConfigLoaderImpl::populate( Configuration& config, const pugi::xml_node& node )
{
    pugi::xpath_node_set list = node.select_nodes( "./Configuration" );

    for ( pugi::xpath_node_set::const_iterator it = list.begin(); it != list.end(); ++it ) {
        Configuration temp;
        if ( load( temp, it->node() ) )
            populate( config.append( temp ), it->node() );
    }
    return false;
}

bool
ConfigLoaderImpl::load( Configuration& config, const pugi::xml_node& node )
{
    if ( std::string( node.name() ) == "Configuration" ) {
        // copy name="my_name"
        config.name( node.attribute( "name" ).value() );

        // config.xml( pugi::helper::to_string( node ) );
        pugi::xml_document dom;
        dom.append_copy( node );
        std::ostringstream o;
        dom.save( o );
        config.xml( o.str() );

        // populate all attributes
        // pugi::xpath_node_set attrs = node.select_nodes( "attribute::*" );
        for ( pugi::xml_attribute_iterator it = node.attributes_begin(); it != node.attributes_end(); ++it )
            config.attribute( it->name(), it->value() );

        pugi::xpath_node title_node = node.select_node( "./title[@lang='jp']" );
        if ( title_node ) {
            config.title( pugi::as_wide( title_node.node().child_value() ) );
        } else {
            if ( ( title_node = node.select_node( "./title[@lang='en']" ) ) )
                config.title( pugi::as_wide( title_node.node().child_value() ) );
            else if ( ( title_node = node.select_node( "./title" ) ) )
                config.title( pugi::as_wide( title_node.node().child_value() ) );
        }

        do {
            std::string interface = node.select_node( "./Component/@interface" ).attribute().value();
            if ( ! interface.empty() )
                config.component_interface( interface );
        } while (0);

        return true;
    }
    return false;
}
