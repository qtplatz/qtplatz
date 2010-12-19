// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "folder.h"
#include "folium.h"

using namespace portfolio;

Folder::~Folder()
{
}

Folder::Folder()
{
}

Folder::Folder( const xmlNode& n ) : Node( n )
{
}

Folder::Folder( const Folder& t ) : Node( t )
{
}

std::vector< Folder >
Folder::folders()
{
    xmlNodeList list = Node::selectNodes( L"./folder[@folderType='directory']" );
    std::vector< Folder > folders;
    for ( size_t i = 0; i < list.size(); ++i )
        folders.push_back( Folder( list[i] ) );
    return folders;
}

Folio
Folder::folio()
{
    xmlNodeList list = Node::selectNodes( L"./folder[@folderType='file']" );
    Folio folio;
    for ( size_t i = 0; i < list.size(); ++i )
        folio.push_back( Folium( list[i] ) );
    return folio;
}

Folium
Folder::selectSingleFolium( const std::wstring& )
{
    return Folium();
}
