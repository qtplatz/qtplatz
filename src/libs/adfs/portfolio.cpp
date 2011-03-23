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
#include "adfs.h"
#include "portfolio.h"
#include "adsqlite.h"

#include "folder.h"
#include "folium.h"
#include "filesystem.h"
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#define BOOST_LIB_NAME boost_filesystem
#include <boost/config/auto_link.hpp>

using namespace adfs;

portfolio::~portfolio()
{
}

portfolio::portfolio()
{
}

portfolio::portfolio( const portfolio& t ) : db_(t.db_)
{
}

std::vector< folder >
portfolio::folders()
{
    std::vector< folder > folders;

    return folders;
    // return impl_->selectFolders( L"./folder[@folderType='directory']" );
}

folium
portfolio::findFolium( const std::wstring& id )
{
    return folium();
    // return impl_->selectFolium( L"//folium[@dataId='" + id + L"']");
}

/////////////
bool
portfolio::create( const wchar_t * filename, size_t alloc, size_t page_size )
{
    boost::filesystem::path filepath( filename );

    if ( boost::filesystem::exists( filepath ) ) {
        boost::system::error_code ec;
        if ( ! boost::filesystem::remove( filepath, ec ) ) {
            throw adfs::exception( ec.message(), ec.category().name() );
            return false;
        }
    }

    db_.reset( new sqlite() );
    if ( db_->open( filepath.c_str() ) ) {
        adfs::stmt sql( *db_ );
        if ( page_size )
            sql.exec( ( boost::format( "PRAGMA page_size = %1%" ) % page_size ).str() );
        if ( alloc )
            internal::fs::prealloc( *db_, alloc );
        return internal::fs::format( *db_, filename );
    }
    db_.reset();
    return false;
}

bool
portfolio::mount( const wchar_t * filename )
{
    if ( db_ )
        db_.reset();

    boost::filesystem::path filepath( filename );

    db_.reset( new sqlite() );
    if ( db_->open( filepath.c_str() ) )
        return internal::fs::mount( *db_ );

    db_.reset();
    return false;
}

folder
portfolio::addFolder( const std::wstring& name, bool uniq )
{
    return internal::fs::add_folder( *db_, name );
}

