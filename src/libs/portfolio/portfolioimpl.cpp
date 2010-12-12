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
    }
}

PortfolioImpl::PortfolioImpl( const PortfolioImpl& t ) : isXMLLoaded_( t.isXMLLoaded_ )
                                                       , NodeIdent( t ) 
                                                       , doc_( t.doc_ )
                                                       , db_( t.db_ ) 
{
}

const std::wstring
PortfolioImpl::fullpath() const
{
    return NodeIdent::attribute( L"fullpath" );
}

