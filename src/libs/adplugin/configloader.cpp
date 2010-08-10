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

	if ( list.size() == 1 ) {
		if ( ConfigLoaderImpl::load( config, list[0] ) )
			ConfigLoaderImpl::populate( config, list[0] );
	} else
		return false;
	return true;
}

/////////////////////////////////////////

bool
ConfigLoaderImpl::populate( Configuration& config, const XMLNode& node )
{
	XMLNodeList list = node.selectNodes( L"./Configuration" );

	for ( int i = 0; size_t(i) < list.size(); ++i ) {
		Configuration temp;
		if ( load( temp, list[i] ) )
			populate( config.append( temp ), list[i] );
	}
	return false;
}

bool
ConfigLoaderImpl::load( Configuration& config, const XMLNode& node )
{
	if ( node.nodeName() == L"Configuration" ) {
		// copy name="my_name"
		config.name( node.attribute( L"name" ) );

		XMLDocument dom;
		dom.appendChild( dom.importNode( const_cast<XMLNode&>(node), true ) );
		config.xml( dom.toString() );
        
#if defined _DEBUG
		std::wstring file = L"C:/Temp/" + config.name() + L".xml";
		dom.save( file );
#endif

		// populate all attributes
		XMLNodeList attrs = node.selectNodes( L"attribute::*" );
		for ( int i = 0; size_t(i) < attrs.size(); ++i )
			config.attribute( attrs[i].nodeName(), attrs[i].textValue() );

		// copy text value
		config.text( node.textValue() );

		XMLNode module_node = node.selectSingleNode( L"./Component[@module]" );
		if ( module_node ) {
			std::wstring module_name = module_node.attribute( L"module" );
			if ( ! module_name.empty() ) {
				std::wstring query = L"//Module[@name=\'" + module_name + L"\']";
				XMLNode module_element = node.selectSingleNode( query );
				if ( module_element ) {
                    XMLDocument dom;
					dom.appendChild( dom.importNode( module_element, true ) );
					internal::xml_element& m = config.module();
					m.xml( dom.toString() );
					XMLNodeList attrs = module_element.selectNodes( L"attribute::*" );
                    for ( int i = 0; size_t(i) < attrs.size(); ++i )
						m.attribute( attrs[i].nodeName(), attrs[i].textValue() );
				}
			}
		}

		return true;
	}
	return false;
}