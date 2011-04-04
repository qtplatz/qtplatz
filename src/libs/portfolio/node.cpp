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

#include "node.h"
#include "portfolioimpl.h"

using namespace portfolio;
using namespace portfolio::internal;

Node::Node() : impl_(0)
{
}

Node::Node( const xmlElement& e, PortfolioImpl* impl ) : impl_(impl), node_(e)
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
        return node_.attribute( key );
    return std::wstring();
}

std::vector< std::pair< std::wstring, std::wstring> >
Node::attributes() const
{
    std::vector< std::pair< std::wstring, std::wstring> > attrs;
    xmlNodeList list = node_.selectNodes( L"attribute::*" );
    for ( int i = 0; i < list.size(); ++i ) {
        std::wstring key = list[i].nodeName();
        std::wstring value = list[i].textValue();
        attrs.push_back( std::make_pair< std::wstring, std::wstring >( key, value ) );
    }
    return attrs;
}

void
Node::setAttribute( const std::wstring& key, const std::wstring& value )
{
    if ( node_ )
        node_.setAttribute( key, value );
}

xmlNodeList
Node::selectNodes( const std::wstring& query )
{
    return node_.selectNodes( query );
}

xmlElement
Node::selectSingleNode( const std::wstring& query )
{
    return node_.selectSingleNode( query );
}

//////////////////////////
xmlElement
Node::addFolder( const std::wstring& name, internal::PortfolioImpl* impl )
{
    xmlElement child = impl->getDocument().createElement( L"folder" );
    node_.appendChild( child );
    child.setAttribute( L"folderType", L"directory" );
    child.setAttribute( L"name", name );
    return child;
}

xmlElement
Node::addFolium( const std::wstring& name )
{
    xmlElement child = impl_->getDocument().createElement( L"folium" );
    node_.appendChild( child );
    child.setAttribute( L"folderType", L"file" );
    child.setAttribute( L"dataId", internal::PortfolioImpl::newGuid() );
    child.setAttribute( L"name", name );
    return child;
}

xmlElement
Node::addAttachment( const std::wstring& name )
{
    xmlElement child = impl_->getDocument().createElement( L"attachment" );
    node_.appendChild( child );
    child.setAttribute( L"dataId", internal::PortfolioImpl::newGuid() );
    child.setAttribute( L"name", name );
    return child;
}

