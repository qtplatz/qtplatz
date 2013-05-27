// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

using namespace adportable;

static std::wstring __error_string;

Configuration::Configuration(void)
{
}

Configuration::~Configuration(void)
{
}

Configuration::Configuration( const Configuration& t ) : xml_( t.xml_ )
						       , name_( t.name_ )
                                                       , text_( t.text_ )
						       , interface_(t.interface_)
                                                       , attributes_( t.attributes_ )
                                                       , children_( t.children_ )  
                                                       , module_( t.module_ )
{
}

const std::wstring&
Configuration::component() const
{
    return attribute( L"component" );
}

const std::wstring&
Configuration::_interface() const
{
    return interface_;  // under <Component> element
}

void
Configuration::interface( const std::wstring& value )
{
    interface_ = value;
}

const std::wstring&
Configuration::attribute( const std::wstring& key ) const
{
    attributes_type::const_iterator it = attributes_.find( key );
    if ( it != attributes_.end() )
        return it->second;
    return __error_string;
}

const std::wstring&
Configuration::name() const
{
    return name_;
}

void
Configuration::name( const std::wstring& value )
{
    name_ = value;
}

const std::wstring&
Configuration::title() const
{
    return text_;
}

void
Configuration::title( const std::wstring& value )
{
    text_ = value;
}

void
Configuration::attribute( const std::wstring& key, const std::wstring& value )
{
    attributes_[ key ] = value;
}

bool
Configuration::readonly() const
{
    if ( attribute( L"readonly" ) == L"true" )
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
Configuration::xml( const std::wstring& xml )
{
    xml_ = xml;
}

void
Configuration::module( const Module& m )
{
    module_ = m;
}

bool
Configuration::isComponent() const
{
    return ! component().empty();
}

bool
Configuration::isPlugin() const
{
    return ! module().xml().empty();
}

// static
const Configuration *
Configuration::find( const Configuration& config, const std::wstring& name )
{
    if ( config.name() == name )
        return &config;
    for ( Configuration::vector_type::const_iterator it = config.begin(); it != config.end(); ++it ) {
        if ( it->name() == name )
            return &(*it);
    }
    const Configuration * p = 0;
    for ( Configuration::vector_type::const_iterator it = config.begin(); it != config.end(); ++it ) {
        if ( ( p = find( *it, name ) ) )
            return p;
    }
    return 0;
}

/////////////////////////////////////

Module::Module( const std::wstring& xml ) : xml_(xml)
{
}

Module::Module( const Module& t ) : xml_( t.xml_ )
                                  , library_filename_( t.library_filename_ )
                                  , object_reference_( t.object_reference_ )
                                  , id_( t.id_ )
{
}

void
Module::xml( const std::wstring& xml )
{
    xml_ = xml;
}

void
Module::library_filename( const std::wstring& name )
{
    library_filename_ = name;
}

void
Module::library_filename( const std::string& name )
{
    library_filename_ = string::convert( name );
}

void
Module::object_reference( const std::string& name )
{
    object_reference_ = name;
}

void
Module::id( const std::string& ident )
{
    id_ = ident;
}

////////////////////
