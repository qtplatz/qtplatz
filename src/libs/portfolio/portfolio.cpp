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

#include "portfolio.hpp"
#include "portfolioimpl.hpp"
#include "folder.hpp"
#include "folium.hpp"
#include <sstream>

#if defined _MSC_VER
# if defined _DEBUG
#    pragma comment(lib, "xmlparserd.lib")
#    pragma comment(lib, "adportabled.lib")
# else
#    pragma comment(lib, "xmlparser.lib")
#    pragma comment(lib, "adportable.lib")
# endif
#endif

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

Portfolio::Portfolio( const std::wstring& xml ) : impl_( new internal::PortfolioImpl( xml ) )
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

std::wstring
Portfolio::xml() const
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
