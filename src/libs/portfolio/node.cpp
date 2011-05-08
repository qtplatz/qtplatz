/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "node.hpp"
#include "portfolioimpl.hpp"

using namespace portfolio;
using namespace portfolio::internal;

Node::Node() : impl_(0)
{
}

Node::Node( const pugi::xml_node& e, PortfolioImpl* impl ) : impl_(impl), node_(e)
{
}

Node::Node( const Node& t ) : impl_(t.impl_), node_( t.node_ )
{
}

Node::operator bool () const 
{
    return impl_;
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

std::wstring
Node::id() const
{
    return attribute( L"dataId" );
}

void
Node::id( const std::wstring& value )
{
   setAttribute( L"dataId", value );
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
        pugi::xml_attribute attr = node_.append_attribute( pugi::as_utf8( key ).c_str() );
        attr.set_value( pugi::as_utf8( value ).c_str() );
    }
}

pugi::xpath_node_set
Node::selectNodes( const std::wstring& query )
{
    return node_.select_nodes( pugi::as_utf8( query ).c_str() );
}

pugi::xpath_node
Node::selectSingleNode( const std::wstring& query )
{
    return node_.select_single_node( pugi::as_utf8( query ).c_str() );
}

//////////////////////////

pugi::xml_node
Node::addFolder( const std::wstring& name, internal::PortfolioImpl* )
{
    pugi::xml_node child = node_.append_child( "folder" );
    child.append_attribute( "folderType" ).set_value( "directory" );
    child.append_attribute( "name" ).set_value( pugi::as_utf8( name ).c_str() );
    return child;
}

pugi::xml_node
Node::addFolium( const std::wstring& name )
{
    // xmlElement child = impl_->getDocument().createElement( L"folium" );
    pugi::xml_node child = node_.append_child( "folium" );
    // node_.appendChild( child );
    child.append_attribute( "folderType" ).set_value( "file" );
    // child.setAttribute( L"folderType", L"file" );
    child.append_attribute( "dataId" ).set_value( pugi::as_utf8( internal::PortfolioImpl::newGuid() ).c_str() );
    // child.setAttribute( L"dataId", internal::PortfolioImpl::newGuid() );
    child.append_attribute( "name" ).set_value( pugi::as_utf8( name ).c_str() );
    // child.setAttribute( L"name", name );
    return child;
}

pugi::xml_node
Node::addAttachment( const std::wstring& name )
{
    // xmlElement child = impl_->getDocument().createElement( L"attachment" );
    pugi::xml_node child = node_.append_child( "attachment" );
    child.append_attribute( "dataId" ).set_value( pugi::as_utf8( internal::PortfolioImpl::newGuid() ).c_str() );
    child.append_attribute( "name" ).set_value( pugi::as_utf8( name ).c_str() );
    //node_.appendChild( child );
    //child.setAttribute( L"dataId", internal::PortfolioImpl::newGuid() );
    //child.setAttribute( L"name", name );
    return child;
}

