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

#include <compiler/disable_unused_parameter.h>

#include "portfolio.hpp"
#include "portfolioimpl.hpp"
#include "folder.hpp"
#include "folium.hpp"
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
Portfolio::findFolder( const std::wstring& name )
{
    return impl_->findFolder( name );
}

Folium
Portfolio::findFolium( const std::wstring& id )
{
    return impl_->selectFolium( L"//folium[@dataId='" + id + L"']");
}

attributes_type
Portfolio::attributes() const
{
    return impl_->attributes();
}

size_t
Portfolio::removed_dataids( std::vector< std::string >& v ) const
{
    v.clear();
    for ( const auto& dataid : impl_->removed_ )
        v.push_back( dataid );
    return v.size();
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

