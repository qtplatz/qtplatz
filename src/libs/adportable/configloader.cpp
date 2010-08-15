//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "ConfigLoader.h"
#include <adportable/configuration.h>
#include <xmlwrapper/msxml.h>

using namespace adportable;
using namespace xmlwrapper;
using namespace xmlwrapper::msxml;

struct ConfigLoaderImpl {
	static bool populate( Configuration&, const XMLNode& );
	static bool load( Configuration&, const XMLNode& );
    static bool resolve_module( Configuration&, const XMLNode& );
};

ConfigLoader::ConfigLoader(void)
{
}

ConfigLoader::~ConfigLoader(void)
{
}

// static
bool
ConfigLoader::loadConfigFile( adportable::Configuration& config, const std::wstring& file, const std::wstring& query )
{
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

// static
bool
ConfigLoader::loadConfigXML( adportable::Configuration& config, const std::wstring& xml, const std::wstring& query )
{
    XMLDocument dom;
	if ( ! dom.loadXML( xml ) )
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
        config.xml( XMLDocument::toString( node ) );

		// populate all attributes
		XMLNodeList attrs = node.selectNodes( L"attribute::*" );
		for ( int i = 0; size_t(i) < attrs.size(); ++i )
			config.attribute( attrs[i].nodeName(), attrs[i].textValue() );

        XMLNode title_node = node.selectSingleNode( L"./title[@lang='jp']" );
        if ( title_node ) {
            config.title( title_node.textValue() );
        } else {
            if ( title_node = node.selectSingleNode( L"./title[@lang='en']" ) )
                config.title( title_node.textValue() );
            else
                config.title( node.textValue() );
        }

        resolve_module( config, node );
		return true;
	}
	return false;
}

bool
ConfigLoaderImpl::resolve_module( Configuration& config, const XMLNode& node )
{
    XMLNode module_attr = node.selectSingleNode( L"./Component/@module" );

    if ( module_attr ) {

		do {
			XMLNode ifattr = node.selectSingleNode( L"./Component/@interface" );
			if ( ifattr )
				config.interface( ifattr.textValue() );
		} while (0);

        std::wstring module_name = module_attr.textValue();
        if ( module_name.empty() )
            return false;

        std::wstring query = L"//Module[@name=\'" + module_name + L"\']";
        XMLNode module_element = node.selectSingleNode( query );

        if ( module_element ) {

            Module module( XMLDocument::toString( module_element ) );
            std::wstring filename = module_element.attribute( L"filename" );
            if ( filename.empty() )
                return false;
            std::wstring::size_type pos = filename.find_last_of( L"$" );
            if ( pos != std::wstring::npos ) {
                filename = filename.substr(0, pos);
#if defined _DEBUG
                filename += L"d.dll";
#else
                filename += L".dll";
#endif         
            }
            module.library_filename( filename );
            config.module( module );

            return true;
        }
    }
    return false;
}

