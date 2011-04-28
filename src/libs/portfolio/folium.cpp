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

#include "folium.h"
#include "folder.h"
#include "portfolioimpl.h"

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

/*
Folium::Folium( xmlNode& n, internal::PortfolioImpl * impl ) : Node( n, impl )
{
}
*/

Folium::Folium( pugi::xml_node& n, internal::PortfolioImpl * impl ) : Node( n, impl )
{
}

std::wstring
Folium::path() const
{
    return attribute( L"path" );
}

bool
Folium::empty() const
{
    if ( impl_ ) {
        boost::any& data = impl_->find( id() );
        return data.empty();
    }
    return true;
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

Folio
Folium::attachments()
{
#if defined USE_MSXML
    xmlNodeList list = Node::selectNodes( L"./attachment" );
    Folio attachments;
    for ( size_t i = 0; i < list.size(); ++i )
        attachments.push_back( Folium( list[i], impl_ ) );
    return attachments;
#else
    Folio attachments;

    pugi::xpath_node_set list = Node::selectNodes( L"./attachment" );
    for ( pugi::xpath_node_set::const_iterator it = list.begin(); it != list.end(); ++it ) 
        attachments.push_back( Folium( it->node(), impl_ ) );

    return attachments;
#endif
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

#if defined USE_MSXML
Folder
Folium::getParentFolder()
{
    // std::wstring query = L"//folder[@folderType='directory']/folium[@dataId=\"" + id() + L"\"]/parent()";
    xmlElement elmt = Node::selectSingleNode( L".." );
    while ( elmt && elmt.attribute( L"folderType" ) != L"directory" )
        elmt = elmt.selectSingleNode( L".." );
    if ( elmt.nodeName() == L"folder" && elmt.attribute( L"folderType" ) == L"directory" )
        return Folder( elmt, impl_ );
    return Folder();
}

#else

Folder
Folium::getParentFolder()
{
    pugi::xml_node parent = node_.parent();
    while ( parent && parent.attribute( "folderType" ).value() != std::string( "directory" ) )
        parent = parent.parent();
    if ( parent.name() == std::string( "folder" )
        && parent.attribute( "folderType" ).value() == std::string( "directory" ) ) 
        return Folder( parent, impl_ );
/*    
    pugi::xpath_node elmt = Node::selectSingleNode( L".." );

    while ( elmt && elmt.attribute( "folderType" ).value() != std::string( "directory") )
        elmt = elmt.selectSingleNode( L".." );
    if ( elmt.nodeName() == L"folder" && elmt.attribute( L"folderType" ) == L"directory" )
        return Folder( elmt, impl_ );
*/
    return Folder();
}
#endif