//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "debug.h"
#include <adportable/string.h>
#include "ConfigLoader.h"
#include <adportable/configuration.h>
#include <xmlwrapper/msxml.h>
#include <xmlwrapper/pugixml.hpp>
#include <xmlwrapper/pugiwrapper.h>
#include <fstream>
#include <boost/foreach.hpp>
// #include <xmlwrapper/contrib/foreach.hpp>

using namespace adportable;
using namespace xmlwrapper;
using namespace xmlwrapper::msxml;

struct ConfigLoaderImpl {

    // msxml
	static bool populate( Configuration&, const XMLNode& );
	static bool load( Configuration&, const XMLNode& );
    static bool resolve_module( Configuration&, const XMLNode& );

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

// static
bool
ConfigLoader::loadConfigFile( adportable::Configuration& config, const std::wstring& file, const std::wstring& query )
{
    XMLDocument dom;
	if ( ! dom.load( file ) ) {
        std::wstring reason = dom.parseError();
		adportable::debug dbg;
        dbg << "adportable::ConfigLoader::loadConfigFile(" << file << ")" << reason;
		return false;
	}

	XMLNodeList list = dom.selectNodes( query );
    if ( list.size() == 0 ) {
		return false;
    }
    if ( list.size() == 0 )
        return false;

	if ( list.size() == 1 ) {
        if ( ConfigLoaderImpl::load( config, list[0] ) )
			ConfigLoaderImpl::populate( config, list[0] );
        return true;
	}

    for ( size_t i = 0; i < list.size(); ++i ) {
        Configuration& child = config.append( Configuration() );
        if ( ConfigLoaderImpl::load( child, list[i] ) )
            ConfigLoaderImpl::populate( child, list[i] );
    }
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

bool
ConfigLoaderImpl::load( Configuration& config, const pugi::xml_node& node )
{
    if ( std::string( node.name() ) == "Configuration" ) {
		// copy name="my_name"
        config.name( pugi::as_wide( node.attribute( "name" ).value() ) );

        config.xml( pugi::helper::to_wstring( node ) );

		// populate all attributes
        pugi::xpath_node_set attrs = node.select_nodes( "attribute::*" );

        for ( pugi::xpath_node_set::const_iterator it = attrs.begin(); it != attrs.end(); ++it )
            config.attribute( pugi::as_wide( it->node().name() ), pugi::as_wide( it->node().child_value() ) );

        pugi::xpath_node title_node = node.select_single_node( "./title[@lang='jp']" );
        if ( title_node ) {
            config.title( pugi::as_wide( title_node.node().child_value() ) );
        } else {
            if ( title_node = node.select_single_node( "./title[@lang='en']" ) )
                config.title( pugi::as_wide( title_node.node().child_value() ) );
            else
                config.title( pugi::as_wide( title_node.node().child_value() ) );
        }

        resolve_module( config, node );
		return true;
	}
	return false;
}


bool
ConfigLoaderImpl::resolve_module( Configuration& config, const pugi::xml_node& node )
{
    pugi::xpath_node module_attr = node.select_single_node( "./Component/@module" );

    if ( module_attr ) {

		do {
            pugi::xpath_node ifattr = node.select_single_node( "./Component/@interface" );
            if ( ifattr )
                config.interface( pugi::as_wide( ifattr.node().child_value() ) );
		} while (0);

        std::string module_name = module_attr.node().child_value(); // child_value() := text_value()
        if ( module_name.empty() )
            return false;

        std::string query = "//Module[@name=\'" + module_name + "\']";
        pugi::xpath_node module_element = node.select_single_node( query.c_str() );

        if ( module_element ) {
            pugi::xml_document tmp;
            tmp.append_copy( module_element.node() );
            std::wostringstream xml;

            tmp.save( xml );

            Module module( xml.str() ); // XMLDocument::toString( module_element ) );

            std::string filename = module_element.node().attribute( "filename" ).value();
            if ( filename.empty() )
                return false;
            std::string::size_type pos = filename.find_last_of( "$" );
            if ( pos != std::wstring::npos ) {
                filename = filename.substr(0, pos);
#if defined _DEBUG
                filename += "d.dll";
#else
                filename += ".dll";
#endif         
            }
            module.library_filename( pugi::as_wide( filename ) );
            config.module( module );

            return true;
        }
    }
    return false;
}



