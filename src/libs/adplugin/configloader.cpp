//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "ConfigLoader.h"
#include <adportable/configuration.h>
#include <adportable/component.h>
#include <xmlwrapper/msxml.h>

using namespace adportable;
using namespace xmlwrapper::msxml;
using namespace adplugin::internal;

struct ConfigLoaderImpl {
	static bool populate( Configuration&, const XMLNode& );
	static bool load( Configuration&, const XMLNode& );
};

ConfigLoader::ConfigLoader(void)
{
}

ConfigLoader::~ConfigLoader(void)
{
}

// static
bool
ConfigLoader::loadConfiguration( adportable::Configuration& config, const std::wstring& file, const std::wstring& query )
{
	using namespace xmlwrapper::msxml;

    XMLDocument dom;
	if ( ! dom.load( file ) )
		return false;

	XMLNodeList list = dom.selectNodes( query );
	if ( list.size() == 0 )
		return false;

	for ( int i = 0; i < int(list.size()); ++i ) {
		Configuration temp;
        const XMLNode& node = list[i];
		if ( ConfigLoaderImpl::load( temp, node ) ) {
			ConfigLoaderImpl::populate( temp, node );
			config.append( temp );
		}
	}
	return true;
}

/////////////////////////////////////////

bool
ConfigLoaderImpl::populate( Configuration& config, const XMLNode& node )
{
    load( config, node );
    
	XMLNodeList list = node.selectNodes( L"./*" );
	for ( int i = 0; i < list.size(); ++i ) {
		Configuration temp;
		if ( load( temp, list[i] ) ) {
			populate( temp, list[i] );
			config.append( temp );
		}
	}
	return false;
}

bool
ConfigLoaderImpl::load( Configuration& config, const XMLNode& node )
{
	// copy name="my_name"
	config.name( node.attribute( L"name" ) );

	// populate all attributes
	XMLNodeList attrs = node.selectNodes( L"attribute::*" );
	for ( int i = 0; i < attrs.size(); ++i )
		config.attribute( attrs[i].nodeName(), attrs[i].textValue() );

	// copy text value
	config.text( node.textValue() );
	return true;
}