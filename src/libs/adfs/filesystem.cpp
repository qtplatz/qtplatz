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
#include "folium.h"
#include "filesystem.h"
#include "adsqlite.h"
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/cstdint.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/tokenizer.hpp>

#if defined WIN32
# include "apiwin32.h"
typedef adfs::detail::win32api impl;
#else
#endif

using namespace adfs;

namespace adfs { namespace internal {

    enum dir_type { type_folder = 1, type_folium = 2, type_attachment = 3 };

    struct dml {
        static bool insert_directory( adfs::sqlite& db, dir_type, boost::int64_t parent_id, const std::wstring& name );
        static boost::int64_t select_directory( adfs::stmt&, dir_type, boost::int64_t parent_id, const std::wstring& name );
        static bool update_mtime( adfs::stmt&, boost::int64_t fileid );
    };

    struct scoped_transaction {
        adfs::stmt& sql_;
        bool commit_;
        scoped_transaction( adfs::stmt& stmt ) : sql_(stmt), commit_(false) { 
            sql_.begin();
        }
        ~scoped_transaction() { 
            commit_ ? sql_.commit() : sql_.rollback();
        }
        bool mtime( boost::int64_t fileid ) { 
            return commit_ = dml::update_mtime( sql_, fileid );
        }
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
            sql.exec( "PRAGMA foreign_keys = ON" );
            if ( alloc )
                prealloc( alloc );
            return internal::fs::format( *db_, filename );
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
            return internal::fs::mount( *db_ );

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
internal::fs::format( adfs::sqlite& db, const std::wstring& filename )
{
    return format_superblock( db, filename ) && 
        format_directory( db );
}

bool
internal::fs::format_superblock( adfs::sqlite& db, const std::wstring& filename )
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
internal::fs::mount( adfs::sqlite& db )
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
internal::dml::insert_directory( adfs::sqlite& db, dir_type type, boost::int64_t parent_id, const std::wstring& name )
{
    adfs::stmt sql( db );

    boost::posix_time::ptime pt = boost::posix_time::microsec_clock::local_time();
    std::string date = ( boost::format( "%1%" ) % pt ).str();

    sql.prepare( "INSERT INTO directory VALUES (?,?,?,?,?,?)" );
    sql.bind( 1 ) = name;
    sql.bind( 2 ) = parent_id; // 
    sql.bind( 3 ) = boost::int64_t( type ); // 1:directory, 2:file
    sql.bind( 4 ) = date;
    sql.bind( 5 ) = date;
    // sql.bind( 6 ) = std::string("");
    return sql.step() == adfs::sqlite_done;
}

boost::int64_t
internal::dml::select_directory( adfs::stmt& sql, dir_type type, boost::int64_t parent_id, const std::wstring& name )
{
    sql.prepare( "SELECT rowid, type, name, parent_id FROM directory WHERE type = ? AND name = ? AND parent_id = ?" );
    sql.bind( 1 ) = static_cast< boost::int64_t>(type);
    sql.bind( 2 ) = name; // name
    sql.bind( 3 ) = parent_id;
    if ( sql.step() == adfs::sqlite_row )
        return boost::get< boost::int64_t >( sql.column_value( 0 ) );
    return 0;
}

bool
internal::dml::update_mtime( adfs::stmt& sql, boost::int64_t fileid )
{
    boost::posix_time::ptime pt = boost::posix_time::microsec_clock::local_time();
    std::string date = ( boost::format( "%1%" ) % pt ).str();

    if ( sql.prepare( "UPDATE directory SET mtime = :mtime WHERE rowid = :fileid" ) ) {
        sql.bind( 1 ) = date;
        sql.bind( 2 ) = fileid;
        return sql.step() == adfs::sqlite_done;
    }
    return false;
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
bool
internal::fs::format_directory( adfs::sqlite& db )
{
    boost::posix_time::ptime pt = boost::posix_time::microsec_clock::local_time();
    std::string date = ( boost::format( "%1%" ) % pt ).str();

    adfs::stmt sql( db );

    sql.exec( "CREATE TABLE directory( \
               name TEXT \
               , parent_id INTEGER \
               , type INTEGER \
               , ctime DATE \
               , mtime DATE \
               , hash INTEGER \
               , UNIQUE( parent_id, name ) \
               )" );
    sql.prepare( "INSERT INTO directory VALUES (?,?,?,?,?,?)" );
    sql.bind( 1 ) = std::string("/"); // root
    sql.bind( 2 ) = 0; // for root
    sql.bind( 3 ) = 1; // 1:directory, 2:file, 3: attachment
    sql.bind( 4 ) = date;
    sql.bind( 5 ) = date;
    sql.bind( 6 ) = std::string("");
    sql.step();
    sql.reset();

    return sql.exec( "CREATE TABLE file(\
                       fileid \
                     , attr BLOB \
                     , data BLOB \
                     , UNIQUE ( fileid ) \
                     , FOREIGN KEY(fileid) REFERENCES directory(rowid) )");
}

bool
internal::fs::prealloc( adfs::sqlite& db, unsigned long long size )
{
    adfs::stmt sql( db );

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

adfs::folder
internal::fs::add_folder( adfs::sqlite& db, const std::wstring& name )
{
    boost::filesystem::path path( name );
    std::wstring branch = path.branch_path().c_str();
    std::wstring leaf = path.leaf().c_str();

    if ( branch.at(0) == L'/' ) { // has to be fullpath

        typedef boost::tokenizer< boost::char_separator<wchar_t>
                                , std::wstring::const_iterator
                                , std::wstring> tokenizer_t;
        boost::char_separator<wchar_t> separator( L"", L"/" );
        tokenizer_t tokens( branch, separator );

        boost::int64_t parent_id = 0, rowid = 0;
        adfs::stmt sql( db );

        for ( tokenizer_t::const_iterator it = tokens.begin(); it != tokens.end(); ++it ) {

            rowid = internal::dml::select_directory( sql, type_folder, parent_id, *it );
            if ( rowid == 0 )
                return adfs::folder(); // error
            parent_id = rowid;
        }

        if ( rowid = internal::dml::select_directory( sql, type_folder, parent_id, leaf ) ) // already exist
            return adfs::folder( db, rowid, leaf );

        if ( internal::dml::insert_directory( db, type_folder, parent_id, leaf ) ) {
            if ( rowid = internal::dml::select_directory( sql, type_folder, parent_id, leaf ) )
                return adfs::folder( db, rowid, leaf );
        }
    }
    return adfs::folder();
}

adfs::folium
internal::fs::add_folium( const folder& folder, const std::wstring& name )
{
    adfs::stmt sql( folder.db() );

    bool validated = false;
    if ( folder.rowid() ) {
        sql.prepare( "SELECT type FROM directory where rowid = ?" );
        sql.bind( 1 ) = folder.rowid();
        if ( sql.step() == sqlite_row && boost::get<boost::int64_t>( sql.column_value( 0 ) ) == type_folder )
            validated = true;
    }

    if ( validated ) {
        // duplicate check
        boost::int64_t rowid;
        if ( rowid = dml::select_directory( sql, type_folium, folder.rowid(), name ) ) // already exist
            return adfs::folium( folder.db(), rowid, name );

        // insert
        dml::insert_directory( folder.db(), type_folium, folder.rowid(), name );
        rowid = dml::select_directory( sql, type_folium, folder.rowid(), name );

        return adfs::folium( folder.db(), rowid, name );
    }

    return adfs::folium();
}

adfs::folium
internal::fs::add_attachment( const folium& parent, const std::wstring& name )
{
    adfs::stmt sql( parent.db() );

    // duplicate check
    boost::int64_t rowid;
    if ( rowid = dml::select_directory( sql, type_attachment, parent.rowid(), name ) ) // already exist
        return adfs::folium( parent.db(), rowid, name, true );

    // insert
    dml::insert_directory( parent.db(), type_attachment, parent.rowid(), name );
    rowid = dml::select_directory( sql, type_attachment, parent.rowid(), name );
    return adfs::folium( parent.db(), rowid, name, true );
}

folder
internal::fs::get_parent_folder( sqlite& db, boost::int64_t rowid )
{
    stmt sql( db );

    sql.prepare( "SELECT rowid, name, type FROM directory WHERE rowid = (SELECT parent_id FROM directory WHERE rowid = ?)" );
    sql.bind( 1 ) = rowid;

    while ( sql.step() == sqlite_row ) {
        boost::int64_t parent_id = boost::get<boost::int64_t>( sql.column_value( 0 ) ); // parent rawid
        std::wstring name = boost::get<std::wstring>( sql.column_value( 1 ) ); // parent name
        dir_type type = static_cast<dir_type>( boost::get<boost::int64_t>( sql.column_value( 2 ) ) );
        if ( type == type_folder )
            return folder( db, parent_id, name );

        sql.reset();
        sql.bind( 1 ) = parent_id;  // recursive search toward upper layer
    }

    return folder();
}

bool
internal::fs::write( adfs::sqlite& db, boost::int64_t fileid, size_t size, const unsigned char * pbuf )
{
    adfs::stmt sql( db );

    scoped_transaction update(sql);

    // fileid should be UNIQUE.
    if ( sql.prepare( "INSERT INTO file (fileid, data) VALUES ( :fileid, :data )" ) ) {
        sql.bind( 1 ) = fileid;
        sql.bind( 2 ) = blob( size, pbuf );
        adfs::step_state res = sql.step();
        if ( res == adfs::sqlite_done && update.mtime( fileid ) )
            return true;

        sql.reset();
        if ( res == adfs::sqlite_error && sql.prepare( "UPDATE file SET data = :data WHERE fileid = :fileid" ) ) {
            sql.bind( 1 ) = blob( size, pbuf );
            sql.bind( 2 ) = fileid;
            return ( sql.step() == adfs::sqlite_done ) && update.mtime( fileid );
        }
    }
    return false;
}
