//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "configuration.h"

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
{
}

const std::wstring&
Configuration::component() const
{
    return attribute( L"component" );
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
Configuration::text() const
{
    return text_;
}

void
Configuration::text( const std::wstring& value )
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

/////////////////////////////////////

using namespace adportable::internal;

xml_element::xml_element()
{
}

xml_element::xml_element( const xml_element& t ) : xml_(t.xml_)
                                                 , text_(t.text_)
												 , attributes_(t.attributes_)  
{
}


void
xml_element::xml( const std::wstring& xml )
{
	xml_ = xml;
}

const std::wstring& 
xml_element::attribute( const std::wstring& key ) const
{
	std::map< std::wstring, std::wstring >::const_iterator it = attributes_.find( key );
    if ( it != attributes_.end() )
		return it->second;
	return __error_string;
}

void
xml_element::attribute( const std::wstring& key, const std::wstring& value )
{
   attributes_[key] = value;
}

void
xml_element::text( const std::wstring& text )
{
	text_ = text;
}

