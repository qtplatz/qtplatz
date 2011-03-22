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

#include "filesystem.h"
#include "adsqlite.h"
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/cstdint.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#if defined WIN32
# include "apiwin32.h"
typedef adfs::detail::win32api impl;
#else
#endif
//#include <sstream>

using namespace adfs;

namespace adfs { namespace internal {
    
    class filesystem {
    public:
        static bool format( sqlite& db, const std::wstring& filename );
        static bool format_superblock( sqlite& db, const std::wstring& filename );
        static bool format_directory( sqlite& db );

        static bool mount( sqlite& db );
    };
}
}

filesystem::filesystem() : db_(0)
{
}

filesystem::~filesystem()
{
    close();
    delete db_;
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

    if ( db_ = new sqlite() ) {
        if ( db_->open( filepath.c_str() ) ) {
            adfs::stmt sql( *db_ );
            if ( page_size )
                sql.exec( ( boost::format( "PRAGMA page_size = %1%" ) % page_size ).str() );
            if ( alloc )
                prealloc( alloc );
            return internal::filesystem::format( *db_, filename );
        }
        delete db_;
        db_ = 0;
    }
    return false;
}

bool
filesystem::mount( const wchar_t * filename )
{
    if ( db_ )
        delete db_;

    boost::filesystem::path filepath( filename );

    if ( db_ = new sqlite() ) {

        if ( db_->open( filepath.c_str() ) )
            return internal::filesystem::mount( *db_ );

        delete db_;
        db_ = 0;
    }
    return false;
}

bool
filesystem::prealloc( size_t size )
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


////////////////////////
bool
internal::filesystem::format( adfs::sqlite& db, const std::wstring& filename )
{
    return format_superblock( db, filename ) && 
        format_directory( db );
}

bool
internal::filesystem::format_superblock( adfs::sqlite& db, const std::wstring& filename )
{
    adfs::stmt sql( db );
    if ( ! sql.exec( 
        "CREATE TABLE superblock( creator TEXT, name TEXT, magic INTEGER, ctime DATE, create_user TEXT )" ) )
        return false;

    boost::posix_time::ptime pt = boost::posix_time::microsec_clock::local_time();
    std::string date = ( boost::format( "%1%" ) % pt ).str();

    sql.prepare( "INSERT INTO superblock VALUES(:creator, :name, :magic, :ctime, :create_user)" );
    sql.bind( 1 ) = std::string("adfs::filesystem version(1.0)"); // creator
    sql.bind( 2 ) = filename;  // unicode
    sql.bind( 3 ) = boost::int64_t( 0x2011031111301102 ); // 2011.03.11-01 
    sql.bind( 4 ) = date; // create_date;
    sql.bind( 5 ) = impl::get_login_name<char>(); // mbcs

    if ( sql.step() == sqlite_done )
        return true;

    return false;
}

bool
internal::filesystem::mount( adfs::sqlite& db )
{
    adfs::stmt sql( db );
    sql.prepare( "SELECT creator, magic from superblock" );

    while ( sql.step() == adfs::sqlite_row ) {

        std::wstring creator = boost::get<std::wstring>( sql.column_value( 0 ) );
        boost::int64_t magic = boost::get<boost::int64_t>( sql.column_value( 1 ) );

        if ( magic == 0x2011031111301102 )
            return true;
    }
    return false;
}


bool
internal::filesystem::format_directory( adfs::sqlite& db )
{
    boost::posix_time::ptime pt = boost::posix_time::microsec_clock::local_time();
    std::string date = ( boost::format( "%1%" ) % pt ).str();

    adfs::stmt sql( db );

    sql.exec( "CREATE TABLE directory( name TEXT, parent_id INTEGER, ctime DATE )" );
    sql.prepare( "INSERT INTO directory VALUES (?,?,?)" );
    sql.bind( 1 ) = std::string("/"); // root
    sql.bind( 2 ) = 0; // for root
    sql.bind( 3 ) = date;
    sql.step();
    sql.reset();

    return sql.exec( "CREATE TABLE file( name TEXT \
                   , directory_id INTEGER \
                   , parent_id INTEGER \
                   , uid TEXT \
                   , ctime DATE \
                   , mtime DATE \
                   , contents BLOB \
                   , data BLOB )");
}
