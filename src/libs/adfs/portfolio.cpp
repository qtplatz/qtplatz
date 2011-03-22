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

#include "portfolio.h"
#include "adsqlite.h"

#include "folder.h"
#include "folium.h"
#include "filesystem.h"
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "internal_filesystem.h"
//# if defined _DEBUG
//#    pragma comment(lib, "xmlwrapperd.lib")
//#    pragma comment(lib, "adportabled.lib")
//# else
//#    pragma comment(lib, "xmlwrapper.lib")
//#    pragma comment(lib, "adportable.lib")
//# endif

using namespace adfs;

Portfolio::~Portfolio()
{
}

Portfolio::Portfolio()
{
}

Portfolio::Portfolio( const Portfolio& t ) : db_(t.db_)
{
}

std::vector< Folder >
Portfolio::folders()
{
    return std::vector< Folder >();
    // return impl_->selectFolders( L"./folder[@folderType='directory']" );
}

Folium
Portfolio::findFolium( const std::wstring& id )
{
    return Folium();
    // return impl_->selectFolium( L"//folium[@dataId='" + id + L"']");
}

/////////////
bool
Portfolio::create( const wchar_t * filename, size_t alloc, size_t page_size )
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
            prealloc( alloc );
        return internal::filesystem::format( *db_, filename );
    }
    db_.reset();
    return false;
}

bool
Portfolio::mount( const wchar_t * filename )
{
    if ( db_ )
        db_.reset();

    boost::filesystem::path filepath( filename );

    db_.reset( new sqlite() );
    if ( db_->open( filepath.c_str() ) )
        return internal::filesystem::mount( *db_ );

    db_.reset();
    return false;
}

bool
Portfolio::prealloc( size_t size )
{
    adfs::stmt sql( *db_ );

    const size_t unit_size = 512 * 1024 * 1024;

    sql.exec( "CREATE TABLE large (a BLOB)" );

    while ( size > unit_size ) {
        sql.exec( "INSERT INTO large VALUES( zeroblob(512 * 1024 * 1024) )" );
        size -= unit_size;
    }
    if ( size )
        sql.exec( ( boost::format( "INSERT INTO large VALUES( zeroblob(%1%) )" ) % size ).str() );

    sql.exec( "DROP TABLE large" );

    return true;
}

/*
bool
Portfolio::create_with_fullpath( const std::wstring& fullpath )
{
    return impl_->create_with_fullpath( fullpath );
}
*/

Folder
Portfolio::addFolder( const std::wstring& name, bool uniq )
{
    return Folder();
    // return impl_->addFolder( name, uniq );
}

/*
std::wstring
Portfolio::xml() const
{
    std::wstring xml;
    // impl_->getDocument().xml( xml );
    return xml;
}

bool
Portfolio::save( const std::wstring& filename ) const
{
    // return impl_->getDocument().save( filename );
    return false;
}
*/