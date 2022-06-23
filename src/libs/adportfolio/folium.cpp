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

#include "folium.hpp"
#include "folder.hpp"
#include "portfolioimpl.hpp"
#include <adportable/debug.hpp>
#include <algorithm>

using namespace portfolio;

Folium::~Folium()
{
}

Folium::Folium()
{
}

Folium::Folium( const Folium& t ) : Node( t )
{
}

Folium::Folium( const pugi::xml_node& n, internal::PortfolioImpl * impl ) : Node( n, impl )
{
}

// Folium::Folium( pugi::xml_node n, internal::PortfolioImpl * impl ) : Node( n, impl )
// {
// }

bool
Folium::empty() const
{
    if ( impl_ ) {
        boost::any& data = impl_->find( id() );
        return data.empty();
    }
    return true;
}

bool
Folium::nil() const
{
    return impl_ == 0;
}

void
Folium::operator = ( const boost::any& any )
{
    if ( impl_ )
        impl_->assign( id(), any );
}

Folium::operator boost::any & ()
{
    if ( impl_ )
        return impl_->find( id() );
    static boost::any temp;
    return temp;
}

Folium::operator const boost::any & () const
{
    if ( impl_ )
        return impl_->find( id() );
    static boost::any temp;
    return temp;
}

void
Folium::assign( const boost::any& any, const wchar_t * dataClass )
{
    if ( impl_ ) {
        impl_->assign( id(), any );
        Node::dataClass( dataClass );
    }
}

Folio
Folium::attachments()
{
    Folio attachments;

    pugi::xpath_node_set list = Node::selectNodes( L"./attachment" );
    for ( pugi::xpath_node_set::const_iterator it = list.begin(); it != list.end(); ++it )
        attachments.emplace_back( Folium( it->node(), impl_ ) );

    return attachments;
}

const Folio
Folium::attachments() const
{
    return const_cast< Folium *>(this)->attachments();
}

Folium
Folium::addAttachment( const std::wstring& name )
{
    return Folium( Node::addAttachment( name ), impl_ );
}

// bool
// Folium::removeAttachment( const std::wstring& name, bool removeContents )
// {
//     if ( Node::removeAttachment( name ) ) {
// 		if ( removeContents )
// 			impl_->collect_garbage();
//         return true;
//     }
//     return false;
// }

bool
Folium::erase_attachment( const std::wstring& name
                          , std::function< void( std::tuple< std::wstring, std::wstring > ) > callback )
{
    pugi::xpath_node_set list = Node::selectNodes( "./attachment[@name='" + pugi::as_utf8( name ) + "']" );
    for ( pugi::xpath_node_set::const_iterator it = list.begin(); it != list.end(); ++it ) {
        Folium child( it->node(), impl_ );
        if ( child.attachments().size() > 0 ) { // if subsidery attachments exists, remove them all
            child.erase_attachments( callback );
        }
        Node::erase( "attachment", { child.name(), child.id() } );
        callback( { name, pugi::as_wide( it->node().attribute( "dataId" ).as_string() ) } );
    }
    return true;
}

bool
Folium::erase_attachments( std::function< void( std::tuple< std::wstring, std::wstring > ) > callback )
{
    pugi::xpath_node_set list = Node::selectNodes( L"./attachment" );
    for ( pugi::xpath_node_set::const_iterator it = list.begin(); it != list.end(); ++it ) {
        Folium child( it->node(), impl_ );
        if ( child.attachments().size() > 0 ) { // recursively call
            child.erase_attachments( callback );
        }
        Node::erase( "attachment", { child.name(), child.id() } );
        callback( { child.name(), child.id() } );
    }
    return true;
}

Folium
Folium::parentFolium() const
{
    pugi::xml_node parent = node_.parent();

    while ( parent && parent.attribute( "folderType" ).value() != std::string( "file" ) )
        parent = parent.parent();

    if ( parent && parent.attribute( "folderType" ).value() == std::string( "file" ) )
        return Folium( parent, impl_ );

    return Folium();
}

Folder
Folium::parentFolder() const
{
    pugi::xml_node parent = node_.parent();

    while ( parent && parent.attribute( "folderType" ).value() != std::string( "directory" ) )
        parent = parent.parent();

    if ( parent && parent.name() == std::string( "folder" )
        && parent.attribute( "folderType" ).value() == std::string( "directory" ) )
        return Folder( parent, impl_ );

    return Folder();
}

std::string
Folium::fullpath( bool fullyqualified ) const
{
	std::vector< std::string > hierachey;

	hierachey.push_back( node_.attribute( "name" ).value() );

	pugi::xml_node parent = node_.parent();

	while ( parent && parent.attribute( "folderType" ).value() != std::string( "directory" ) )
		parent = parent.parent();

	while ( parent.name() == std::string( "folder" )
		&& parent.attribute( "folderType" ).value() == std::string( "directory" ) ) {
		hierachey.push_back( parent.attribute( "name" ).value() );
		parent = parent.parent();
	}
	std::string path;
	if ( fullyqualified ) {
		pugi::xpath_node dset = node_.select_node( "/xtree/dataset" );
		path = dset.node().attribute( "fullpath" ).value();
		path += "::";
	}
	std::for_each( hierachey.rbegin(), hierachey.rend(), [&path]( const std::string& name ){
		path += "/" + name;
	});
	return path;
}
