//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "nodeident.h"

using namespace portfolio;
using namespace portfolio::internal;

NodeIdent::NodeIdent()
{
}

NodeIdent::NodeIdent( const xmlElement& e ) : node_(e)
{
}

NodeIdent::NodeIdent( const NodeIdent& t ) : node_( t.node_ )
{
}

std::wstring
NodeIdent::name() const
{
    return attribute( L"name" );
}

void
NodeIdent::name( const std::wstring& value )
{
    setAttribute( L"name", value );
}

std::wstring
NodeIdent::id() const
{
    return attribute( L"dataId" );
}

void
NodeIdent::id( const std::wstring& value )
{
   setAttribute( L"dataId", value );
}

bool
NodeIdent::isFolder() const
{
    return attribute( L"folderType" ) == L"directory";
}

void
NodeIdent::isFolder( bool )
{
    setAttribute( L"folderType", L"directory" );
}

std::wstring
NodeIdent::dataClass() const
{
    return attribute( L"dataType" );
}

void
NodeIdent::dataClass( const std::wstring& value )
{
    setAttribute( L"dataType", value );
}

std::wstring
NodeIdent::attribute( const std::wstring& key ) const
{
    if ( node_ )
        return node_.attribute( key );
    return std::wstring();
}

void
NodeIdent::setAttribute( const std::wstring& key, const std::wstring& value )
{
    if ( node_ )
        node_.setAttribute( key, value );
}