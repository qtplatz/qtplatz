//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

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
       vec.push_back( Folder( list[i] ) );
    return vec;
}