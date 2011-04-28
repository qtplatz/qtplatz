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
    pugi::xml_parse_result result = doc_.load( pugi::as_utf8( xml ).c_str() );
    if ( result ) {
        pugi::xpath_node node = doc_.select_single_node( "/xtree/dataset" );
        if ( node ) {
            node_ = node.node();
            isXMLLoaded_ = true;
#if defined _DEBUG && 0
            doc_.save_file( "portfolio-pugi.xml" );
#endif
        }
    }
}

PortfolioImpl::PortfolioImpl( const PortfolioImpl& t ) : isXMLLoaded_( t.isXMLLoaded_ )
                                                       , Node( t ) 
#if defined USE_MSXMS
                                                       , doc_( t.doc_ )
#endif
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

#if defined USE_MSXML
    xmlNodeList list = Node::selectNodes( query );
    for ( size_t i = 0; i < list.size(); ++i )
       vec.push_back( Folder( list[i], this ) );
#else
    pugi::xpath_node_set list = Node::selectNodes( query );
    for ( pugi::xpath_node_set::const_iterator it = list.begin(); it != list.end(); ++it ) {
        vec.push_back( Folder ( it->node(), this ) );
    }
#endif
    return vec;
}

Folium
PortfolioImpl::selectFolium( const std::wstring& query )
{
#if defined USE_MSXMS
    xmlNodeList list = Node::selectNodes( query );
    if ( list.size() )
        return Folium( list[0], this );
#else
    pugi::xpath_node_set list = Node::selectNodes( query );
    if ( list.size() )
        return Folium( list[0].node(), this );
#endif

    return Folium(); // empty
}

boost::any&
PortfolioImpl::find( const std::wstring& id )
{
    return db_[ id ];  // this will create a node if not exist
}

void
PortfolioImpl::assign( const std::wstring& id, const boost::any& data )
{
    db_[ id ] = data;
}

//////////////////////

bool
PortfolioImpl::create_with_fullpath( const std::wstring& fullpath )
{
    if ( isXMLLoaded_ )
        return false;

#if defined USE_MSXML
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
#else

    pugi::xml_node inst = doc_.append_child( pugi::node_pi );  // processingInstruction
    inst.set_name( "xml" );
    inst.set_value( "version='1.0' encoding='UTF-8'" );

/*
    XMLProcessingInstruction inst =
        doc_.createProcessingInstruction(L"xml", L"version='1.0' encoding='UTF-8'");
    doc_.appendChild( inst );
    */  
    pugi::xml_node comm = doc_.append_child( pugi::node_comment );
    comm.set_value( "Copyright(C) 2010-2011, Toshinobu Hondo, ScienceLiaison, All rights reserved." );
/*
    XMLComment comm =
        doc_.createComment(L"Copyright(C) 2010-2011, Toshinobu Hondo, ScienceLiaison, All rights reserved.");
    doc_.appendChild( comm );
*/
    pugi::xml_node top = doc_.append_child();
    top.set_name( "xtree" );
    top.append_attribute( "typeid" ).set_value( "portfolio" );
    top.append_attribute( "impl" ).set_value( "portfolio" );
    top.append_attribute( "create_date" ).set_value( "TBD" );

    pugi::xml_node dset = top.append_child();
    dset.set_name( "dataset" );
    dset.append_attribute( "fullpath" ).set_value( pugi::as_utf8( fullpath ).c_str() );
/*
    XMLElement top = doc_.createElement( L"xtree" );
    doc_.appendChild( top );

    top.setAttribute( L"typeid", L"portfolio" );
    top.setAttribute( L"impl", L"portfolio" );
    top.setAttribute( L"create_date", L"TBD" );

    XMLElement dset = top.appendChild( doc_.createElement( L"dataset" ) );
    dset.setAttribute( L"fullpath", fullpath );
*/
    pugi::xpath_node dataset = doc_.select_single_node( "/xtree/dataset" );
    if ( dataset ) {
        node_ = dataset.node();
        isXMLLoaded_ = true;
    }
#endif
    return isXMLLoaded_;
}

Folder
PortfolioImpl::addFolder( const std::wstring& name, bool uniq )
{
#if defined USE_MSXML
    if ( uniq ) {
        xmlNodeList list = Node::selectNodes( L"folder[@folderType='directory'][@name=\"" + name + L"\"]" );
        if ( list.size() > 0 )
            return Folder( list[0], this );
    }
#else

    if ( uniq ) {
        pugi::xpath_node_set list = Node::selectNodes( L"folder[@folderType='directory'][@name=\"" + name + L"\"]" );
        if ( list.size() > 0 )
            return Folder( list[0].node(), this );
    }

#endif
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
