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

#include "portfolioimpl.h"

using namespace adfs;
using namespace adfs::internal;

#if 0
PortfolioImpl::PortfolioImpl() : db_(0)
{
}

PortfolioImpl::PortfolioImpl( const std::wstring& xml ) // : isXMLLoaded_(false)
{
/*
    if ( doc_.loadXML( xml ) ) {
        if ( node_ = doc_.selectSingleNode( L"/xtree/dataset" ) )
            isXMLLoaded_ = true;
#ifdef _DEBUG
        doc_.save( L"portfolio.xml" );
#endif
    }
*/
}

PortfolioImpl::PortfolioImpl( const PortfolioImpl& t ) : db_( t.db_ ) 
                                                       , Node( t ) 
{
}

const std::wstring
PortfolioImpl::fullpath() const
{
    return Node::attribute( L"fullpath" );
}

std::vector<Folder>
PortfolioImpl::selectFolders( const std::wstring& query )
{
    std::vector<Folder> vec;
    
    xmlNodeList list = Node::selectNodes( query );
    for ( size_t i = 0; i < list.size(); ++i )
       vec.push_back( Folder( list[i], this ) );
    return vec;
}

Folium
PortfolioImpl::selectFolium( const std::wstring& query )
{
    xmlNodeList list = Node::selectNodes( query );
    if ( list.size() )
        return Folium( list[0], this );
    return Folium(); // empty
}

boost::any&
PortfolioImpl::find( const std::wstring& id )
{
    return db_[ id ];  // this will create a node if not exist
}

void
PortfolioImpl::assign( const std::wstring& id, boost::any& data )
{
    db_[ id ] = data;
}

//////////////////////

bool
PortfolioImpl::create_with_fullpath( const std::wstring& fullpath )
{
    if ( isXMLLoaded_ )
        return false;

    using namespace xmlwrapper::msxml;

    XMLProcessingInstruction inst =
        doc_.createProcessingInstruction(L"xml", L"version='1.0' encoding='UTF-8'");
    doc_.appendChild( inst );
    
    XMLComment comm =
        doc_.createComment(L"Copyright(C) 2010-2011, Toshinobu Hondo, ScienceLiaison, All rights reserved.");
    doc_.appendChild( comm );

    XMLElement top = doc_.createElement( L"xtree" );
    doc_.appendChild( top );

    top.setAttribute( L"typeid", L"portfolio" );
    top.setAttribute( L"impl", L"portfolio" );
    top.setAttribute( L"create_date", L"TBD" );

    XMLElement dset = top.appendChild( doc_.createElement( L"dataset" ) );
    dset.setAttribute( L"fullpath", fullpath );

    if ( node_ = doc_.selectSingleNode( L"/xtree/dataset" ) )
        isXMLLoaded_ = true;

    return isXMLLoaded_;
}

Folder
PortfolioImpl::addFolder( const std::wstring& name, bool uniq )
{
    if ( uniq ) {
        xmlNodeList list = Node::selectNodes( L"folder[@folderType='directory'][@name=\"" + name + L"\"]" );
        if ( list.size() > 0 )
            return Folder( list[0], this );
    }
    return Folder( Node::addFolder( name, this ), this );
}



#include <windows.h>
std::wstring
PortfolioImpl::newGuid()
{
#if defined WIN32
    std::wstring guidString;
    GUID guid;
    if ( CoCreateGuid( &guid ) == S_OK ) {
        LPOLESTR psz;
        if ( ::StringFromCLSID( guid, &psz ) == S_OK ) {
            guidString = psz;
            CoTaskMemFree( psz );
        }
    }
    return guidString;
#endif
}
#endif