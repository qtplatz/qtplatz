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

#include "portfolioimpl.h"
#include "folder.h"
#include "folium.h"

using namespace portfolio;
using namespace portfolio::internal;

// Portfolio/PortofolioImpl is corresponding to xtree<CDatafolder> class on libmc4
// It is representing dataset, or exported data

PortfolioImpl::PortfolioImpl() : isXMLLoaded_(false)
{
}

PortfolioImpl::PortfolioImpl( const std::wstring& xml ) : isXMLLoaded_(false)
{
    if ( doc_.loadXML( xml ) ) {
        if ( node_ = doc_.selectSingleNode( L"/xtree/dataset" ) )
            isXMLLoaded_ = true;
#ifdef _DEBUG
        doc_.save( L"portfolio.xml" );
#endif
    }
}

PortfolioImpl::PortfolioImpl( const PortfolioImpl& t ) : isXMLLoaded_( t.isXMLLoaded_ )
                                                       , Node( t ) 
                                                       , doc_( t.doc_ )
                                                       , db_( t.db_ ) 
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