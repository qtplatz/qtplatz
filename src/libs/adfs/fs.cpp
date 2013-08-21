// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "fs.hpp"
#include "adfs.hpp"
#include "file.hpp"
#include "sqlite.hpp"
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
//#include <compiler/disable_unused_parameter.h>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/tokenizer.hpp>
#include <adportable/utf.hpp>
#include <adportable/debug.hpp>

#if defined WIN32
# include "apiwin32.hpp"
  typedef adfs::detail::win32api impl;
#else
# include "apiposix.hpp"
  typedef adfs::detail::posixapi impl;
#endif

namespace adfs {
    namespace internal {

        struct to_posix_time {
            static boost::posix_time::ptime ptime( const adfs::column_value_type& v ) {
                return boost::posix_time::time_from_string( adportable::utf::to_utf8( boost::get<std::wstring>(v) ) );
            }
        };

    }
}

using namespace adfs;
using namespace adfs::internal;

bool
fs::format( adfs::sqlite& db, const std::wstring& filename )
{
    return format_superblock( db, filename ) && format_directory( db );
}

bool
fs::format_superblock( adfs::sqlite& db, const std::wstring& filename )
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
    sql.bind( 3 ) = 0x2011031111301102LL; // 2011.03.11-01 
    sql.bind( 4 ) = date; // create_date;
    sql.bind( 5 ) = impl::get_login_name<char>(); // mbcs

    if ( sql.step() == sqlite_done )
        return true;

    return false;
}

bool
fs::mount( adfs::sqlite& db )
{
    adfs::stmt sql( db );
    sql.prepare( "SELECT creator, magic from superblock" );

    while ( sql.step() == adfs::sqlite_row ) {

        std::wstring creator = boost::get<std::wstring>( sql.column_value( 0 ) );
        int64_t magic = boost::get<int64_t>( sql.column_value( 1 ) );

        if ( magic == 0x2011031111301102LL )
            return true;
    }
    return false;
}

bool
fs::format_directory( adfs::sqlite& db )
{
    boost::posix_time::ptime pt = boost::posix_time::microsec_clock::local_time();
    std::string date = ( boost::format( "%1%" ) % pt ).str();

    adfs::stmt sql( db );

    if ( sql.exec( "CREATE TABLE directory( \
                    name TEXT \
                    , parent_id INTEGER \
                    , type INTEGER \
                    , ctime DATE \
                    , mtime DATE \
                    , hash TEXT \
                    , attr BLOB \
                    , UNIQUE( parent_id, name ) )" ) ) {

        if ( ! internal::dml::insert_directory( db, type_folder, 0, L"/" ) )
            throw adfs::exception( "format directory failed: can't insert root directory", "sqlite3" );
        sql.reset();
    }

    // fileid can duplicate for attachment
    return sql.exec( "CREATE TABLE file(\
                       fileid \
                     , attr BLOB \
                     , data BLOB \
                     , UNIQUE (fileid) \
                     , FOREIGN KEY(fileid) REFERENCES directory(rowid) )");
}

bool
fs::prealloc( adfs::sqlite& db, unsigned long long size )
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
fs::add_folder( adfs::sqlite& db, const std::wstring& name, bool recursive )
{
    boost::filesystem::path path( name );
    std::wstring branch = path.branch_path().wstring(); //.c_str();
    std::wstring leaf = path.leaf().wstring(); //c_str();

    if ( branch.at(0) == L'/' ) { // has to be fullpath

        typedef boost::tokenizer< boost::char_separator<wchar_t>
                                , std::wstring::const_iterator
                                , std::wstring> tokenizer_t;
        boost::char_separator<wchar_t> separator( L"", L"/" );
        tokenizer_t tokens( branch, separator );

        int64_t parent_id = 0, rowid = 0;

        for ( tokenizer_t::const_iterator it = tokens.begin(); it != tokens.end(); ++it ) {
			// adfs::stmt sql( db );
			rowid = internal::dml::select_directory( db, type_folder, parent_id, *it );
            if ( rowid == 0 ) {
				if ( recursive && internal::dml::insert_directory( db, type_folder, parent_id, *it ) )
					rowid = internal::dml::select_directory( db, type_folder, parent_id, *it );
				else
					return adfs::folder(); // error
			}
            parent_id = rowid;
        }

		rowid = internal::dml::select_directory( db, type_folder, parent_id, leaf );
        if ( rowid ) // already exist
            return adfs::folder( db, rowid, leaf );

        if ( internal::dml::insert_directory( db, type_folder, parent_id, leaf ) ) {
			if ( ( rowid = internal::dml::select_directory( db, type_folder, parent_id, leaf ) ) )
                return adfs::folder( db, rowid, leaf );
        }
    }
    return adfs::folder();
}

adfs::folder
fs::find_folder( adfs::sqlite& db, const std::wstring& name )
{
    boost::filesystem::path path( name );
    std::wstring branch = path.branch_path().wstring(); // .c_str();
    std::wstring leaf = path.leaf().wstring(); // c_str();

    if ( branch.at(0) == L'/' ) { // has to be fullpath

        typedef boost::tokenizer< boost::char_separator<wchar_t>
                                , std::wstring::const_iterator
                                , std::wstring> tokenizer_t;
        boost::char_separator<wchar_t> separator( L"", L"/" );
        tokenizer_t tokens( branch, separator );

        int64_t parent_id = 0, rowid = 0;
		// adfs::stmt sql( db );

        for ( tokenizer_t::const_iterator it = tokens.begin(); it != tokens.end(); ++it ) {

			rowid = internal::dml::select_directory( db, type_folder, parent_id, *it );
            if ( rowid == 0 )
                return adfs::folder(); // error
            parent_id = rowid;
        }

		rowid = internal::dml::select_directory( db, type_folder, parent_id, leaf );
        if ( rowid ) // find it
            return adfs::folder( db, rowid, leaf );

    }
    return adfs::folder();
}

adfs::file
fs::add_file( const folder& folder, const std::wstring& name )
{
    return dml::insert_file( folder.db(), type_file, folder.rowid(), name );
}

adfs::file
fs::add_attachment( const file& parent, const std::wstring& name )
{
    return dml::insert_file( parent.db(), type_attachment, parent.rowid(), name );
}

folder
fs::get_parent_folder( sqlite& db, int64_t rowid )
{
    stmt sql( db );

    sql.prepare( "SELECT rowid, name, type FROM directory WHERE rowid = (SELECT parent_id FROM directory WHERE rowid = ?)" );
    sql.bind( 1 ) = rowid;

    while ( sql.step() == sqlite_row ) {
        int64_t parent_id = boost::get<int64_t>( sql.column_value( 0 ) ); // parent rawid
        std::wstring name = boost::get<std::wstring>( sql.column_value( 1 ) ); // parent name
        dir_type type = static_cast<dir_type>( boost::get<int64_t>( sql.column_value( 2 ) ) );
        if ( type == type_folder )
            return folder( db, parent_id, name );

        sql.reset();
        sql.bind( 1 ) = parent_id;  // recursive search toward upper layer
    }

    return folder();
}

bool
fs::write( adfs::sqlite& db, int64_t rowid, size_t size, const char_t * pbuf )
{
    adfs::stmt sql( db );

    // fileid should be UNIQUE.
    if ( sql.prepare( "UPDATE file SET data = :data WHERE fileid = :rowid" ) ) {
        sql.bind( 1 ) = blob( size, reinterpret_cast< const int8_t *> (pbuf) );
        sql.bind( 2 ) = rowid;
        return ( sql.step() == adfs::sqlite_done );
    }
    return false;
}


bool
fs::read( adfs::sqlite& db, int64_t rowid, size_t size, char_t * pbuf )
{
    adfs::blob blob;
    return blob.open( db, "main", "file", "data", rowid, adfs::readonly )
        && blob.read( reinterpret_cast<int8_t*>(pbuf), size );
}

std::size_t
fs::size( adfs::sqlite& db, int64_t rowid )
{
    adfs::blob blob;
    if ( blob.open( db, "main", "file", "data", rowid, adfs::readonly ) )
        return blob.size();
    return 0;
}

int64_t
fs::rowid_from_fileid( adfs::sqlite& db, int64_t fileid )
{
    adfs::stmt sql( db );
    if ( sql.prepare( "SELECT rowid FROM file WHERE fileid = :fileid" ) ) {
        sql.bind( 1 ) = fileid;
        if ( sql.step() == sqlite_row )
            return boost::get<int64_t>( sql.column_value( 0 ) );
    }
    return 0;
}

bool
fs::select_folders( sqlite& db, int64_t parent_id, std::vector<folder>& vec )
{
    stmt sql( db );

    if ( sql.prepare( "SELECT rowid, name, ctime, mtime FROM directory WHERE type = 1 AND parent_id = :parent_id" ) ) {

        sql.bind( 1 ) = parent_id;

        while ( sql.step() == sqlite_row ) {
            int64_t rowid = boost::get<int64_t>( sql.column_value( 0 ) );
            std::wstring name = boost::get<std::wstring>( sql.column_value( 1 ) );
            try {
                boost::posix_time::ptime ctime = to_posix_time::ptime( sql.column_value( 2 ) );
                boost::posix_time::ptime mtime = to_posix_time::ptime( sql.column_value( 3 ) );
                (void)ctime;
                (void)mtime;
            } catch ( std::out_of_range& ex ) {
                adportable::debug(__FILE__, __LINE__) << "Outof range error: " << ex.what();
                assert(0);
            }
            vec.push_back( folder(db, rowid, name) );
        }
    }
    return true;  
}

bool
fs::select_file( sqlite& db, int64_t parent_id, const std::wstring& id, adfs::file& file )
{
    stmt sql( db );

    if ( sql.prepare( "SELECT rowid, name, ctime, mtime FROM directory WHERE type = 2 AND name = :name AND parent_id = :parent_id" ) ) {

        sql.bind( 1 ) = id; // name (uuid)
		sql.bind( 2 ) = parent_id;

        if ( sql.step() == sqlite_row ) {

            int64_t fileid = boost::get<int64_t>( sql.column_value( 0 ) );
            std::wstring name = boost::get<std::wstring>( sql.column_value( 1 ) );
            try {
                boost::posix_time::ptime ctime = to_posix_time::ptime( sql.column_value( 2 ) );
                boost::posix_time::ptime mtime = to_posix_time::ptime( sql.column_value( 3 ) );
                (void)ctime;
                (void)mtime;
            } catch ( std::out_of_range& ex ) {
                adportable::debug(__FILE__, __LINE__) << "Outof range error: " << ex.what();
                assert(0);
            }
            file = adfs::file( db, fileid, name );
            return true;
        }
    }
    return false;
}

bool
fs::select_files( sqlite& db, int64_t parent_id, files& files )
{
    stmt sql( db );

    if ( sql.prepare( "SELECT rowid, name, ctime, mtime FROM directory WHERE type = 2 AND parent_id = :parent_id order by rowid asc" ) ) {

        sql.bind( 1 ) = parent_id;

        while ( sql.step() == sqlite_row ) {

            int64_t rowid = boost::get<int64_t>( sql.column_value( 0 ) );
            std::wstring name = boost::get<std::wstring>( sql.column_value( 1 ) );
            try {
                boost::posix_time::ptime ctime = to_posix_time::ptime( sql.column_value( 2 ) );
                boost::posix_time::ptime mtime = to_posix_time::ptime( sql.column_value( 3 ) );
                (void)ctime;
                (void)mtime;
            } catch ( std::out_of_range& ex ) {
                adportable::debug(__FILE__, __LINE__) << "Outof range error: " << ex.what();
                assert(0);
            }
            files.push_back( file( db, rowid, name ) );
        }
        return true;
    }
    return false;
}

/////////////////////
bool
internal::dml::insert_directory( adfs::sqlite& db, dir_type type, int64_t parent_id, const std::wstring& name )
{
    adfs::stmt sql( db );

    boost::posix_time::ptime pt = boost::posix_time::microsec_clock::local_time();
    std::string date = ( boost::format( "%1%" ) % pt ).str();

    if ( sql.prepare( "INSERT INTO directory VALUES (?,?,?,?,?,NULL,NULL)" ) ) {
        sql.bind( 1 ) = adportable::utf::to_utf8( name );
        sql.bind( 2 ) = parent_id; // 
        sql.bind( 3 ) = int64_t( type ); // 1:directory, 2:file
        sql.bind( 4 ) = date;
        sql.bind( 5 ) = date;
        return sql.step() == adfs::sqlite_done;
    }
    return false;
}

int64_t
internal::dml::select_directory( adfs::sqlite& db, dir_type type, int64_t parent_id, const std::wstring& name )
{
    adfs::stmt sql( db );
    sql.prepare( "SELECT rowid, type, name, parent_id FROM directory WHERE type = ? AND name = ? AND parent_id = ?" );
    sql.bind( 1 ) = static_cast< int64_t>(type);
    sql.bind( 2 ) = adportable::utf::to_utf8( name ); // name
    sql.bind( 3 ) = parent_id;
    if ( sql.step() == adfs::sqlite_row )
        return boost::get< int64_t >( sql.column_value( 0 ) );
    return 0;
}

bool
internal::dml::update_mtime( adfs::stmt& sql, int64_t fileid )
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

adfs::file
internal::dml::insert_file( adfs::sqlite& db, dir_type type, int64_t parentid, const std::wstring& name )
{
    int64_t fileid(0);

    // find or create entry on directory
	if ( (fileid = dml::select_directory( db, type, parentid, name )) == 0 ) {
        if ( dml::insert_directory( db, type, parentid, name ) )
            fileid = dml::select_directory( db, type, parentid, name );
    }

	adfs::stmt sql( db );
    if ( sql.prepare( "INSERT INTO file (fileid) VALUES ( :fileid )" ) ) {  // might be error due to unique constraints
        sql.bind( 1 ) = fileid;

        sqlite_state res = sql.step();
        if ( res == sqlite_done || res == sqlite_constraint )
            return adfs::file( db, fileid, name );
    }
    return adfs::file();
}

