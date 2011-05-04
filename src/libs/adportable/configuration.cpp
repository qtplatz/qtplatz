//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "configuration.hpp"

using namespace adportable;

static std::wstring __error_string;

Configuration::Configuration(void)
{
}

Configuration::~Configuration(void)
{
}

Configuration::Configuration( const Configuration& t ) : name_( t.name_ )
                                                       , text_( t.text_ )
                                                       , attributes_( t.attributes_ )
                                                       , children_( t.children_ )  
													   , xml_( t.xml_ )
                                                       , module_( t.module_ )
													   , interface_(t.interface_)
{
}

const std::wstring&
Configuration::component() const
{
    return attribute( L"component" );
}

const std::wstring&
Configuration::interface() const
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
        if ( p = find( *it, name ) )
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

////////////////////