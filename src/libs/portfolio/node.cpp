//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "node.h"

using namespace portfolio;
using namespace portfolio::internal;

Node::Node()
{
}

Node::Node( const xmlElement& e ) : node_(e)
{
}

Node::Node( const Node& t ) : node_( t.node_ )
{
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