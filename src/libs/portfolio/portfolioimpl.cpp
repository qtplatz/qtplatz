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

#if defined _MSC_VER
# pragma warning( disable: 4996 )
#endif
#include "portfolioimpl.hpp"
#include "folder.hpp"
#include "folium.hpp"
#include <adportable/debug.hpp>
#include <adportable/utf.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <compiler/diagnostic_push.h>
#include <compiler/disable_unused_parameter.h>
#include <boost/lexical_cast.hpp>
#include <compiler/diagnostic_pop.h>

using namespace portfolio;
using namespace portfolio::internal;

// Portfolio/PortofolioImpl is corresponding to xtree<CDatafolder> class on libmc4
// It is representing dataset, or exported data

PortfolioImpl::PortfolioImpl() : isXMLLoaded_(false)
{
}

PortfolioImpl::PortfolioImpl( const std::string& xml ) : isXMLLoaded_(false)
{
    pugi::xml_parse_result result = doc_.load( xml.c_str() );
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

PortfolioImpl::PortfolioImpl( const PortfolioImpl& t ) : Node( t ) 
                                                       , isXMLLoaded_( t.isXMLLoaded_ )
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


    pugi::xpath_node_set list = Node::selectNodes( query );
    for ( pugi::xpath_node_set::const_iterator it = list.begin(); it != list.end(); ++it )
        vec.push_back( Folder ( it->node(), this ) );

    return vec;
}

Folium
PortfolioImpl::selectFolium( const std::wstring& query )
{
    pugi::xpath_node_set list = Node::selectNodes( query );
    if ( list.size() )
        return Folium( list[0].node(), this );

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

    doc_.reset();
    // pugi::xml_node inst = doc_.append_child( pugi::node_pi );  // processingInstruction
    // inst.set_name( "xml" );
    // inst.set_value( "version='1.0' encoding='UTF-8'" );

    pugi::xml_node comm = doc_.append_child( pugi::node_comment );
    comm.set_value( "Copyright(C) 2010-2014, MS-Cheminformatics LLC, All rights reserved." );

    // create "/xtree/dataset" entry
    pugi::xml_node top = doc_.append_child();
    top.set_name( "xtree" );
    top.append_attribute( "typeid" ).set_value( "portfolio" );
    top.append_attribute( "impl" ).set_value( "portfolio" );
    top.append_attribute( "create_date" ).set_value( "TBD" );

    pugi::xml_node dset = top.append_child();
    dset.set_name( "dataset" );
    dset.append_attribute( "fullpath" ).set_value( pugi::as_utf8( fullpath ).c_str() );

    pugi::xpath_node dataset = doc_.select_single_node( "/xtree/dataset" );
    if ( dataset ) {
        node_ = dataset.node();
        isXMLLoaded_ = true;
    }

    return isXMLLoaded_;
}

Folder
PortfolioImpl::addFolder( const std::wstring& name, bool uniq )
{
    using adportable::utf;
    if ( uniq ) {
        std::string query = std::string( "folder[@folderType='directory'][@name=\"" ) + utf::to_utf8(name) + "\"]";
        try {
            pugi::xpath_node_set list = Node::selectNodes( query );
            if ( list.size() > 0 )
                return Folder( list[0].node(), this );
        } catch ( pugi::xpath_exception& ex ) {
            adportable::debug(__FILE__, __LINE__) << "xml_exception: " << ex.what();
            assert(0);
        }
    }
    return Folder( Node::addFolder( name, this ), this );
}

Folder
PortfolioImpl::findFolder( const std::wstring& name )
{
    using adportable::utf;
    std::string query = std::string( "folder[@folderType='directory'][@name=\"" ) + utf::to_utf8(name) + "\"]";
    try {
        pugi::xpath_node_set list = Node::selectNodes( query );
        if ( list.size() > 0 )
            return Folder( list[0].node(), this );
    } catch ( pugi::xpath_exception& ex ) {
        adportable::debug(__FILE__, __LINE__) << "xml_exception: " << ex.what();
    }
    return Folder();
}

std::wstring
PortfolioImpl::newGuid()
{
    const boost::uuids::uuid id = boost::uuids::random_generator()();
    std::string s = boost::lexical_cast<std::string>(id);
	return pugi::as_wide( s );
}

bool
PortfolioImpl::collect_garbage()
{
    std::vector< std::wstring > candidates;

    for ( auto a: db_ ) {
        std::string query = "//*[@dataId=\"" + pugi::as_utf8( a.first ) +"\"]";
        try {
            pugi::xpath_node node = node_.select_single_node( query.c_str() );
            if ( ! node )
				candidates.push_back( a.first );
        } catch ( pugi::xpath_exception& ex ) {
            adportable::debug(__FILE__, __LINE__) << "xml_exception: " << ex.what();
            assert(0);            
        }
    }
	for ( auto garbage: candidates )
		db_.erase( garbage );
	return true;
}
