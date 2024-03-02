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

#include "portfolio.hpp"
#include "portfolioimpl.hpp"
#include "folder.hpp"
#include "folium.hpp"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sstream>

using namespace portfolio;

Portfolio::~Portfolio()
{
}

Portfolio::Portfolio() : impl_( new internal::PortfolioImpl() )
{
}

Portfolio::Portfolio( const Portfolio& t ) : impl_(t.impl_)
{
}

Portfolio::Portfolio( const std::string& xml ) : impl_( new internal::PortfolioImpl( xml ) )
{
}

Portfolio::Portfolio( const std::wstring& xml ) : impl_( new internal::PortfolioImpl( pugi::as_utf8( xml ) ) )
{
}

Portfolio::Portfolio( std::shared_ptr< internal::PortfolioImpl > t ) : impl_( t )
{
}

std::vector< Folder >
Portfolio::folders()
{
    return impl_->selectFolders( L"./folder[@folderType='directory']" );
}

const std::vector< Folder >
Portfolio::folders() const
{
    return impl_->selectFolders( L"./folder[@folderType='directory']" );
}

Folder
Portfolio::findFolder( const std::wstring& name ) const
{
    return impl_->findFolder( name );
}

Folium
Portfolio::findFolium( const std::wstring& id ) const
{
    return impl_->selectFolium( L"//folium[@dataId='" + id + L"']");
}

Folium
Portfolio::findFolium( const boost::uuids::uuid& id ) const
{
    std::wostringstream o;
    o << L"//folium[@dataId='" << id << L"']";
    return impl_->selectFolium( o.str() );
}


template<> std::vector< std::pair< std::wstring, std::wstring > >
Portfolio::attributes< std::wstring >() const
{
    return impl_->attributes< std::wstring >();
}

template<> std::vector< std::pair< std::string, std::string > >
Portfolio::attributes< std::string >() const
{
    return impl_->attributes< std::string >();
}

// attributes_type
// Portfolio::attributes() const
// {
//     return impl_->attributes();
// }

const std::vector< std::tuple< std::string, std::string > >&
Portfolio::erased_dataIds() const
{
    return impl_->removed_list_;
}

/////////////

bool
Portfolio::create_with_fullpath( const std::wstring& fullpath )
{
    return impl_->create_with_fullpath( fullpath );
}

Folder
Portfolio::addFolder( const std::wstring& name, bool uniq )
{
    return impl_->addFolder( name, uniq );
}

Folder
Portfolio::addFolder( const std::string& name, bool uniq )
{
    return impl_->addFolder( name, uniq );
}


std::string
Portfolio::xml() const
{
    std::ostringstream o;
    impl_->getDocument().save( o );
    return o.str();
}

std::wstring
Portfolio::wxml() const
{
    std::wostringstream o;
    impl_->getDocument().save( o );
    return o.str();
}

bool
Portfolio::save( const std::wstring& filename ) const
{
    return impl_->getDocument().save_file( filename.c_str() );
}

std::wstring
Portfolio::fullpath() const
{
	return impl_->fullpath();
}

boost::uuids::uuid
Portfolio::uuidFromString( const std::string& id )
{
    return internal::Node::uuidFromString( id );
}

boost::uuids::uuid
Portfolio::uuidFromString( const std::wstring& id )
{
    return internal::Node::uuidFromString( pugi::as_utf8( id ));
}
