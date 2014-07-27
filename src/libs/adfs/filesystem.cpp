// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "adfs.hpp"
#include "file.hpp"
#include "filesystem.hpp"
#include "sqlite.hpp"
#include "fs.hpp"
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <adportable/debug.hpp>

using namespace adfs;

filesystem::~filesystem()
{
    close();
}

filesystem::filesystem() : format_version_(0)
{
}

filesystem::filesystem( const filesystem& t ) : db_( t.db_ )
                                              , format_version_(t.format_version_)
{
}

bool
filesystem::close()
{
    return db_ && db_->close();
}

bool
filesystem::create( const wchar_t * filename, size_t alloc, size_t page_size )
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

        if ( internal::fs::format( *db_, filename, format_version_ ) ) {
            sql.exec( "PRAGMA FOREIGN_KEYS = ON" );
            return true;
        }
    }
    db_.reset();

    return false;
}

bool
filesystem::mount( const wchar_t * filename )
{
    boost::filesystem::path filepath( filename );

    db_.reset( new sqlite() );

    if ( db_->open( filepath.c_str() ) ) {

        if ( internal::fs::mount( *db_, format_version_ ) ) {

            if ( format_version_ >= 3 ) {
                adfs::stmt sql( *db_ );
                sql.exec( "PRAGMA FOREIGN_KEYS = ON" );
            }

            return true;
        }
    }

    db_.reset();
    return false;
}

folder
filesystem::addFolder( const std::wstring& name, bool recursive )
{
    return internal::fs::add_folder( *db_, name, recursive );
}

folder
filesystem::findFolder( const std::wstring& name ) const
{
    return internal::fs::find_folder( *db_, name );
}

file
filesystem::findFile( const folder& folder, const std::wstring& id )
{
    file file;
    if ( db_ )
		internal::fs::select_file( *db_, folder.rowid(), id, file );
    return file;
}

////////////////////////

