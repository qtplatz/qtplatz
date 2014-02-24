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

#include "folder.hpp"
#include "folium.hpp"
#include "portfolioimpl.hpp"

using namespace portfolio;

Folder::~Folder()
{
}

Folder::Folder()
{
}

Folder::Folder( const pugi::xml_node& n, internal::PortfolioImpl * impl ) : Node( n, impl )
{
}

Folder::Folder( const Folder& t ) : Node( t )
{
}

bool
Folder::nil() const
{
    return impl_ == 0;
}

std::vector< Folder >
Folder::folders()
{
    std::vector< Folder > folders;

    pugi::xpath_node_set list = node_.select_nodes( "./folder[@folderType='directory']" );
    for ( pugi::xpath_node_set::const_iterator it = list.begin(); it != list.end(); ++it )
        folders.push_back( Folder( it->node(), impl_ ) );

    return folders;
}

const std::vector< Folder >
Folder::folders() const
{
    return const_cast< Folder * >(this)->folders();
}


Folio
Folder::folio()
{
    Folio folio;

    pugi::xpath_node_set list = node_.select_nodes( "./folder[@folderType='file']|./folium" );
    for ( pugi::xpath_node_set::const_iterator it = list.begin(); it != list.end(); ++it )
        folio.push_back( Folium( it->node(), impl_ ) );

    return folio;
}

const Folio
Folder::folio() const
{
    return const_cast< Folder * >(this)->folio();
}

Folium
Folder::findFoliumByName( const std::wstring& name )
{
	std::string query = "./folium[@name=\"" + pugi::as_utf8( name ) + "\"]";
    pugi::xpath_node node = node_.select_single_node( query.c_str() );
    if ( node.node().empty() )
        return Folium();
	return Folium( node.node(), impl_ );
}

Folium
Folder::findFoliumById( const std::wstring& id )
{
	std::string query = "./folium[@id=\"" + pugi::as_utf8( id ) + "\"]";
    pugi::xpath_node node = node_.select_single_node( query.c_str() );
    if ( node.node().empty() )
        return Folium();
	return Folium( node.node(), impl_ );
}

/////////////////////////
Folium
Folder::addFolium( const std::wstring& name )
{
    return Folium( Node::addFolium( name ), impl_ );
}

bool
Folder::removeFolium( const Folium& folium )
{
    if ( Node::removeFolium( folium.id() ) ) {
        impl_->collect_garbage();
        return true;
    }
    return false;
}
