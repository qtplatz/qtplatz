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

#include "configuration.hpp"
#include "string.hpp"
#include <algorithm>

using namespace adportable;

static std::string __error_string;

Configuration::Configuration(void)
{
}

Configuration::~Configuration(void)
{
}

Configuration::Configuration( const Configuration& t ) : xml_( t.xml_ )
                                                       , title_( t.title_ )
                                                       , component_interface_(t.component_interface_)
                                                       , attributes_( t.attributes_ )
                                                       , children_( t.children_ )  
{
}

Configuration&
Configuration::operator = ( const Configuration& t )
{
    xml_ = t.xml_;
    title_ = t.title_;
    component_interface_ = t.component_interface_;
    attributes_ = t.attributes_;
    children_ = t.children_;
	return *this;
}

const std::string&
Configuration::component() const
{
    return attribute( "component" );
}

const std::string&
Configuration::component_interface() const
{
    return component_interface_;  // under <Component> element
}

void
Configuration::component_interface( const std::string& value )
{
    component_interface_ = value;
}

const std::string&
Configuration::attribute( const std::string& key ) const
{
    auto it = attributes_.find( key );
    if ( it != attributes_.end() )
        return it->second;
    return __error_string;
}

const std::string&
Configuration::name() const
{
    return attribute( "name" );
}

void
Configuration::name( const std::string& value )
{
    attribute( "name", value );
}

const std::wstring&
Configuration::title() const
{
    return title_;
}

void
Configuration::title( const std::wstring& value )
{
    title_ = value;
}

void
Configuration::attribute( const std::string& key, const std::string& value )
{
    attributes_[ key ] = value;
}

bool
Configuration::readonly() const
{
    if ( attribute( "readonly" ) == "true" )
        return true;
    return false;
}

bool
Configuration::hasChild() const
{
    return ! children_.empty();
}

Configuration&
Configuration::append( const Configuration& t )
{
    children_.push_back( t );
    return children_.back();
}

void
Configuration::xml( const std::string& xml )
{
    xml_ = xml;
}

// static
const Configuration *
Configuration::find( const Configuration& config, const std::string& name )
{
	// check first layer
    if ( config.name() == name )
        return &config;
	const auto it = std::find_if( config.begin(), config.end(), [&]( const Configuration& c ){
		return c.name() == name;
	});
	if ( it != config.end() )
		return &(*it);

	// recursive search
    const Configuration * p = 0;
	for ( const Configuration& c: config ) {
        if ( ( p = find( c, name ) ) )
            return p;
    }
    return 0;
}

/////////////////////////////////////

