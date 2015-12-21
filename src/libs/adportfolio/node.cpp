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

#if _MSC_VER
# pragma warning(disable:4996)
#endif
#include "node.hpp"
#include "portfolioimpl.hpp"
#include <adportable/debug.hpp>
#include <adportable/utf.hpp>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <string>

#include <compiler/diagnostic_push.h>
#include <compiler/disable_unused_parameter.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <compiler/diagnostic_pop.h>

using namespace portfolio;
using namespace portfolio::internal;

namespace portfolio { namespace internal {

    static void set_attribute( pugi::xml_node& node, const char * key, const std::string& value ) {
        pugi::xml_attribute attrib = node.attribute( key );
        if ( ! attrib )
            attrib = node.append_attribute( key );
        attrib.set_value( value.c_str() );
    }

}
}

boost::uuids::uuid
Node::uuidFromString( const std::string& id )
{
    boost::uuids::uuid uuid = { 0 };
    if ( !id.empty() ) {
        try {
            if ( id[ 0 ] == '{' ) { // Windows CreateGUID format
                auto idstr = id.substr( 1, 36 );
                uuid = boost::lexical_cast<boost::uuids::uuid>( idstr );
                return uuid;
            } else {
                uuid = boost::lexical_cast<boost::uuids::uuid>( id );
                return uuid;
            }
        } catch ( boost::bad_lexical_cast& ) {
            ADDEBUG() << "bad uuid cast: '" << id << "'";
        }
    }
    return uuid;
}

Node::Node() : impl_(0)
             , uuid_({ 0 })
{
}

Node::Node( const pugi::xml_node& e, PortfolioImpl* impl ) : node_( e )
                                                           , impl_( impl )
{
    std::string id = node_.attribute( "dataId" ).value();
    uuid_ = uuidFromString( id );
}

Node::Node( const Node& t ) : node_( t.node_ )
                            , impl_( t.impl_ )
                            , uuid_( t.uuid_ )
{
}

Node::operator bool () const 
{
    return impl_ != 0;
}

std::wstring
Node::name() const
{
    return attribute( L"name" );
}

void
Node::name( const std::wstring& value )
{
    setAttribute( L"name", value );
}

const boost::uuids::uuid&
Node::uuid() const
{
    return uuid_;
}

std::wstring
Node::id() const
{
    return attribute( L"dataId" );
}

void
Node::id( const std::wstring& value )
{
   setAttribute( L"dataId", value );
   uuid_ = uuidFromString( pugi::as_utf8( value ) );
}

bool
Node::isFolder() const
{
    return attribute( L"folderType" ) == L"directory";
}

void
Node::isFolder( bool f )
{
    if ( f )
        setAttribute( L"folderType", L"directory" );
}

std::wstring
Node::dataClass() const
{
    return attribute( L"dataType" );
}

void
Node::dataClass( const std::wstring& value )
{
    setAttribute( L"dataType", value );
}

std::wstring
Node::attribute( const std::wstring& key ) const
{
    if ( node_ )
        return pugi::as_wide( node_.attribute( pugi::as_utf8( key ).c_str() ).value() );
    return std::wstring();
}

std::vector< std::pair< std::wstring, std::wstring> >
Node::attributes() const
{
    std::vector< std::pair< std::wstring, std::wstring > > attrs;

    using pugi::as_wide;

    for ( pugi::xml_attribute_iterator it = node_.attributes_begin(); it != node_.attributes_end(); ++it )
        attrs.push_back( std::make_pair<std::wstring, std::wstring>( as_wide( it->name() ), as_wide( it->value() ) ) );

    return attrs;
}

void
Node::setAttribute( const std::wstring& key, const std::wstring& value )
{
    if ( node_ ) {
        pugi::xml_attribute attrib = node_.attribute( pugi::as_utf8( key ).c_str() );
        if ( ! attrib )
            attrib = node_.append_attribute( pugi::as_utf8( key ).c_str() );
        attrib.set_value( pugi::as_utf8( value ).c_str() );
    }
}

pugi::xpath_node_set
Node::selectNodes( const std::wstring& query )
{
    try {
        return node_.select_nodes( pugi::as_utf8( query ).c_str() );
    } catch ( pugi::xpath_exception& ex ) {
        ADDEBUG() << "xml_exception: " << ex.what();
        BOOST_THROW_EXCEPTION( ex );
    }
    return pugi::xpath_node_set();
}

pugi::xpath_node_set
Node::selectNodes( const std::string& query )
{
    try {
        return node_.select_nodes( query.c_str() );
    } catch ( pugi::xpath_exception& ex ) {
        ADDEBUG() << "xml_exception: " << ex.what();
        BOOST_THROW_EXCEPTION( ex );
    }
    return pugi::xpath_node_set();
}

pugi::xpath_node
Node::selectSingleNode( const std::wstring& query )
{
    try {
        return node_.select_single_node( pugi::as_utf8( query ).c_str() );
    } catch ( pugi::xpath_exception& ex ) {
        ADDEBUG() << "xml_exception: " << ex.what();
        BOOST_THROW_EXCEPTION( ex );
    }
    return pugi::xpath_node();
}

pugi::xpath_node
Node::selectSingleNode( const std::string& query )
{
    return node_.select_single_node( query.c_str() );
}

//////////////////////////

pugi::xml_node
Node::addFolder( const std::wstring& name, internal::PortfolioImpl* )
{
    pugi::xml_node child = node_.append_child( "folder" );
    set_attribute( child, "folderType", "directory" );
    set_attribute( child, "name", pugi::as_utf8( name ) );
    return child;
}

pugi::xml_node
Node::addFolium( const std::wstring& name )
{
    pugi::xml_node child = node_.append_child( "folium" );
    set_attribute( child, "folderType", "file" );
    set_attribute( child, "dataId", pugi::as_utf8( internal::PortfolioImpl::newGuid() ) );
    set_attribute( child, "name", pugi::as_utf8( name ) );

    boost::posix_time::ptime pt = boost::posix_time::microsec_clock::local_time();
    std::string date = ( boost::format( "%1%" ) % pt ).str();
    set_attribute( child, "ctime", date.c_str() );

    return child;
}

bool
Node::removeFolium( const std::wstring& id )
{
	std::string query = "./folium[@dataId=\"" + pugi::as_utf8( id ) + "\"]";
    try {
        pugi::xpath_node_set nodes = node_.select_nodes( query.c_str() );
        for ( pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it ) {
            node_.remove_child( it->node() );
            impl_->removed( it->node().attribute( "dataId" ).as_string() );
        }
        impl_->removed( pugi::as_utf8(id) );
        return !nodes.empty();
    } catch ( pugi::xpath_exception& ex ) {
        ADDEBUG() << "xml_exception: " << ex.what();
        BOOST_THROW_EXCEPTION( ex );
    }
    return false;
}

pugi::xml_node
Node::addAttachment( const std::wstring& name, bool bUniq )
{
    if ( bUniq ) {
        using adportable::utf;
        std::string query = "./attachment[@name=\"" + utf::to_utf8( name ) + "\"]";
        try {
            pugi::xpath_node_set nodes = node_.select_nodes( query.c_str() );
            for ( pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it ) {
                node_.remove_child( it->node() );
                impl_->removed( it->node().attribute( "dataId" ).as_string() );
            }
        } catch ( pugi::xpath_exception& ex ) {
            ADDEBUG() << "xml_exception: " << ex.what();
            BOOST_THROW_EXCEPTION( ex );
        }
    }

    pugi::xml_node child = node_.append_child( "attachment" );
    set_attribute( child, "dataId", pugi::as_utf8( internal::PortfolioImpl::newGuid() ) );
    set_attribute( child, "name", pugi::as_utf8( name ) );

    boost::posix_time::ptime pt = boost::posix_time::microsec_clock::local_time();
    std::string date = ( boost::format( "%1%" ) % pt ).str();
    set_attribute( child, "ctime", date.c_str() );

    return child;
}

bool
Node::removeAttachment( const std::wstring& name )
{
	std::string query = "./attachment[@name=\"" + pugi::as_utf8( name ) + "\"]";
    try {
        pugi::xpath_node_set nodes = node_.select_nodes( query.c_str() );
        for ( pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it ) {
            node_.remove_child( it->node() );
            impl_->removed( it->node().attribute( "dataId" ).as_string() );
        }
        return !nodes.empty();
    } catch ( pugi::xpath_exception& ex ) {
        ADDEBUG() << "xml_exception: " << ex.what();
        BOOST_THROW_EXCEPTION( ex );
    }
    return false;
}

