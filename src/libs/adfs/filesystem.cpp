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
    return create( std::filesystem::path( filename ), alloc, page_size );
}

bool
filesystem::create( const char * filename, size_t alloc, size_t page_size )
{
    return create( std::filesystem::path( filename ), alloc, page_size );
}

bool
filesystem::create( const std::filesystem::path& filepath, size_t alloc, size_t page_size )
{
    if ( std::filesystem::exists( filepath ) ) {
        std::error_code ec;
        if ( ! std::filesystem::remove( filepath, ec ) ) {
            return false;
        }
        filename_ = filepath.string();
    }

    db_.reset( new sqlite() );
    if ( db_->open( filepath.c_str() ) ) {

        filename_ = filepath.string();

        adfs::stmt sql( *db_ );

        sql.exec( "PRAGMA synchronous = OFF" );
        sql.exec( "PRAGMA journal_mode = MEMORY" );

        if ( page_size )
            sql.exec( ( boost::format( "PRAGMA page_size = %1%" ) % page_size ).str() );

        if ( alloc )
            internal::fs::prealloc( *db_, alloc );

        if ( internal::fs::format( *db_, filepath.wstring(), format_version_ ) ) {

            db_->set_fs_format_version( format_version_ );

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
    return mount( std::filesystem::path( filename ) );
}

bool
filesystem::mount( const char * filename )
{

    return mount( std::filesystem::path( filename ) );
}

bool
filesystem::mount( const std::filesystem::path& filepath )
{
    db_.reset( new sqlite() );

    if ( db_->open( filepath.c_str() ) ) {

        filename_ = filepath.string();

        if ( internal::fs::mount( *db_, format_version_ ) ) {

            db_->set_fs_format_version( format_version_ );

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
filesystem::root() const
{
    stmt sql( *db_ );
    if ( sql.prepare( "SELECT rowid,name FROM directory WHERE type=1 AND parent_id=0" ) ) {
        if ( sql.step() == sqlite_row ) {
            return adfs::folder( *db_
                                 , sql.get_column_value< int64_t >( 0 )          // rowid (fileid)
                                 , sql.get_column_value< std::wstring >( 1 ) );  // name
        }
    }
    return adfs::folder();
}

folder
filesystem::addFolder( const std::wstring& name, bool recursive )
{
    std::lock_guard< std::mutex > lock( mutex_ );
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

const std::string&
filesystem::filename() const
{
    return filename_;
}
////////////////////////
