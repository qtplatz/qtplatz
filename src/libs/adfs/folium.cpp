// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

using namespace adfs;

folium::~folium()
{
}

folium::folium()
{
}


folium::folium( const folium& t ) : Node( t ) 
{
}

/*
folium::folium( xmlNode& n, internal::PortfolioImpl * impl ) : Node( n, impl )
{
}
*/

std::wstring
folium::path() const
{
    return attribute( L"path" );
}

bool
folium::empty() const
{
/*
    if ( impl_ ) {
        boost::any& data = impl_->find( id() );
        return data.empty();
    }
*/
    return true;
}

void
folium::operator = ( boost::any& any )
{
/*
    if ( impl_ )
        impl_->assign( id(), any );
*/
}

folium::operator boost::any & ()
{
/*
    if ( impl_ )
        return impl_->find( id() );
*/
    static boost::any temp;
    return temp;
}

folio
folium::attachments()
{
    // xmlNodeList list = Node::selectNodes( L"./attachment" );
    folio attachments;
/*
    for ( size_t i = 0; i < list.size(); ++i )
        attachments.push_back( folium( list[i], impl_ ) );
*/
    return attachments;
}

folium
folium::addAttachment( const std::wstring& name )
{
    // return folium( Node::addAttachment( name ), impl_ );
    return folium();
}

folder
folium::getParentFolder()
{
    // std::wstring query = L"//folder[@folderType='directory']/folium[@dataId=\"" + id() + L"\"]/parent()";
/*
    xmlElement elmt = Node::selectSingleNode( L".." );
    while ( elmt && elmt.attribute( L"folderType" ) != L"directory" )
        elmt = elmt.selectSingleNode( L".." );
    if ( elmt.nodeName() == L"folder" && elmt.attribute( L"folderType" ) == L"directory" )
        return Folder( elmt, impl_ );
*/
    return folder();
}
