/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC
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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <string>


using namespace portfolio;
using namespace portfolio::internal;

namespace portfolio {
    namespace internal {

        static void set_attribute(pugi::xml_node &node
                                  , const char *key,
                                  const std::string &value)   {
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
    boost::uuids::uuid uuid = { {0} };
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

Node::Node() : impl_({})
             , uuid_({ {0} })
{
}

Node::Node( const pugi::xml_node& e, std::shared_ptr< PortfolioImpl > impl ) : node_( e )
                                                                             , impl_( impl )
{
    std::string id = node_.attribute( "dataId" ).value();
    uuid_ = uuidFromString( id );
    // validation
    if ( ! id.empty() ) {
        std::ostringstream o;
        o << uuid_;
        if ( id != o.str() ) {
            ADDEBUG() << "===== Warning: uuid and dataId did not match: " << std::make_pair( id, uuid_ ) << " -- attribute fixed to uuid";
            setAttribute( "dataId", o.str() ); // make sure uuid and string id match, since old data generated on Windows has CreateGUID format value
        }
    }
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

template<>
std::wstring
Node::name() const
{
    return attribute( L"name" );
}

template<>
std::string
Node::name() const
{
    return attribute( "name" );
}

void
Node::name( const std::wstring& value )
{
    setAttribute( L"name", value );
}

void
Node::name( const std::string& value )
{
    setAttribute( "name", value );
}

const boost::uuids::uuid&
Node::uuid() const
{
    return uuid_;
}

template<> std::wstring
Node::id() const
{
    return attribute( L"dataId" ); // identical to uuid, but wstring
}

template<> std::string
Node::id() const
{
    return attribute( "dataId" );  // identical to uuid, but string
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

template<> std::string Node::dataClass() const       { return attribute( "dataType" ); }
template<> std::wstring Node::dataClass() const      { return attribute( L"dataType" ); }

// std::wstring
// Node::dataClass() const
// {
//     return attribute( L"dataType" );
// }

void
Node::dataClass( const std::wstring& value )
{
     setAttribute( L"dataType", value );
}

void
Node::dataClass( const std::string& value )
{
     setAttribute( "dataType", value );
}


std::wstring
Node::attribute( const std::wstring& key ) const
{
    if ( node_ )
        return pugi::as_wide( node_.attribute( pugi::as_utf8( key ).c_str() ).value() );
    return {};
}

std::string
Node::attribute( const std::string& key ) const
{
    if ( node_ )
        return node_.attribute( key.c_str() ).value();
    return {};
}

template<>
std::vector< std::pair< std::wstring, std::wstring > >
Node::attributes< std::wstring >() const
{
    std::vector< std::pair< std::wstring, std::wstring > > attrs;

    using pugi::as_wide;

    for ( pugi::xml_attribute_iterator it = node_.attributes_begin(); it != node_.attributes_end(); ++it )
        attrs.emplace_back( as_wide( it->name() ), as_wide( it->value() ) );

    return attrs;
}

template<>
std::vector< std::pair< std::string, std::string > >
Node::attributes< std::string >() const
{
    std::vector< std::pair< std::string, std::string > > attrs;

    for ( pugi::xml_attribute_iterator it = node_.attributes_begin(); it != node_.attributes_end(); ++it )
        attrs.emplace_back( it->name(), it->value() );

    return attrs;
}

std::string
Node::attributes_json() const
{
    boost::json::object obj;
    for ( pugi::xml_attribute_iterator it = node_.attributes_begin(); it != node_.attributes_end(); ++it )
        obj[ it->name() ] = it->value();
    return boost::json::serialize( boost::json::object {{ "attributes", obj }} );
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

void
Node::setAttribute( const std::string& key, const std::string& value )
{
    if ( node_ ) {
        pugi::xml_attribute attrib = node_.attribute( key.c_str() );
        if ( ! attrib )
            attrib = node_.append_attribute( key.c_str() );
        attrib.set_value( value.c_str() );
    }
}

template<> void
Node::appendAttributes( const std::vector< std::pair<std::wstring, std::wstring> >& v, bool dontOverride )
{
    const auto& attrs = attributes< std::wstring >();
    for ( const auto& a: v ) {
        if ( dontOverride ) {
            auto it = std::find_if( attrs.begin(), attrs.end(), [&](const auto& x){ return x.first == a.first; } );
            if ( it == attrs.end() )
                setAttribute( a.first, a.second );
        } else {
            setAttribute( a.first, a.second );
        }
    }
}

template<> void
Node::appendAttributes( const std::vector< std::pair<std::string, std::string> >& v, bool dontOverride )
{
    const auto& attrs = attributes< std::string >();
    for ( const auto& a: v ) {
        if ( dontOverride ) {
            auto it = std::find_if( attrs.begin(), attrs.end(), [&](const auto& x){ return x.first == a.first; } );
            if ( it == attrs.end() )
                setAttribute( a.first, a.second );
        } else {
            setAttribute( a.first, a.second );
        }
    }
}

pugi::xpath_node
Node::select_node( const std::string& query )
{
    return node_.select_node( query.c_str() );
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
        return node_.select_node( pugi::as_utf8( query ).c_str() );
    } catch ( pugi::xpath_exception& ex ) {
        ADDEBUG() << "xml_exception: " << ex.what();
        BOOST_THROW_EXCEPTION( ex );
    }
    return pugi::xpath_node();
}

pugi::xpath_node
Node::selectSingleNode( const std::string& query )
{
    return node_.select_node( query.c_str() );
}

//////////////////////////
pugi::xml_node
Node::addFolder( const std::wstring& name, internal::PortfolioImpl* impl )
{
    return addFolder( pugi::as_utf8( name ), impl );
}

pugi::xml_node
Node::addFolder( const std::string& name, internal::PortfolioImpl* )
{
    pugi::xml_node child = node_.append_child( "folder" );
    set_attribute( child, "folderType", "directory" );
    set_attribute( child, "name", name );
    return child;
}

pugi::xml_node
Node::addFolium( const std::wstring& name )
{
    return addFolium( pugi::as_utf8( name ) );
}

pugi::xml_node
Node::addFolium( const std::string& name )
{
    pugi::xml_node child = node_.append_child( "folium" );
    set_attribute( child, "folderType", "file" );
    set_attribute( child, "dataId", pugi::as_utf8( internal::PortfolioImpl::newGuid() ) );
    set_attribute( child, "name", name );

    boost::posix_time::ptime pt = boost::posix_time::microsec_clock::local_time();
    std::string date = ( boost::format( "%1%" ) % pt ).str();
    set_attribute( child, "ctime", date.c_str() );

    return child;
}

pugi::xml_node
Node::addAttachment( const std::wstring& name, bool bUniq )
{
    return addAttachment( pugi::as_utf8( name ), bUniq );
}

pugi::xml_node
Node::addAttachment( const std::string& name, bool bUniq )
{
    if ( bUniq ) {
        using adportable::utf;
        std::string query = "./attachment[@name=\"" + name + "\"]";
        try {
            pugi::xpath_node_set nodes = node_.select_nodes( query.c_str() );
            for ( pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it ) {
                impl_->erase_data( it->node().attribute( "name" ).as_string(), it->node().attribute( "dataId" ).as_string() );
                node_.remove_child( it->node() );
            }
        } catch ( pugi::xpath_exception& ex ) {
            ADDEBUG() << "xml_exception: " << ex.what();
            BOOST_THROW_EXCEPTION( ex );
        }
    }

    pugi::xml_node child = node_.append_child( "attachment" );
    set_attribute( child, "dataId", pugi::as_utf8( internal::PortfolioImpl::newGuid() ) );
    set_attribute( child, "name", name );

    boost::posix_time::ptime pt = boost::posix_time::microsec_clock::local_time();
    std::string date = ( boost::format( "%1%" ) % pt ).str();
    set_attribute( child, "ctime", date.c_str() );

    return child;
}

template<> std::string
Node::filename() const
{
    if ( impl_ )
        return impl_->attribute( "fullpath" );
    return {};
}

template<> std::wstring
Node::filename() const
{
    if ( impl_ )
        return impl_->attribute( L"fullpath" );
    return {};
}

std::vector< std::string >
Node::erase( const std::string& node // "folium" | "attachment"
             , std::tuple< std::wstring, std::wstring > ids )
{
    std::vector< std::string > dataIds;
    auto query = ( boost::format( "./%1%[@name='%2%' and @dataId='%3%']" )
                   % node % pugi::as_utf8( std::get< 0 >(ids) ) % pugi::as_utf8( std::get< 1 >(ids) ) ).str();
    // ADDEBUG() << "-------> erase: " << query;
    try {
        pugi::xpath_node_set nodes = node_.select_nodes( query.c_str() );
        for ( pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it ) {
            // ADDEBUG() << "\t== remove_child(" << it->node().attribute( "dataId" ).as_string() << ") -- ok";
            dataIds.emplace_back( it->node().attribute( "dataId" ).as_string() );
            impl_->erase_data( it->node().attribute( "name" ).as_string(), it->node().attribute( "dataId" ).as_string() );
            node_.remove_child( it->node() );
        }
        return dataIds;
    } catch ( pugi::xpath_exception& ex ) {
        ADDEBUG() << "xml_exception: " << ex.what();
        BOOST_THROW_EXCEPTION( ex );
    }
    return {};
}
