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

#pragma once

#include "adfs_global.h"
#include <string>
#include <cstdint>
#include <functional>

struct sqlite3;
struct sqlite3_stmt;
struct sqlite3_blob;

namespace adfs {

    enum flags { readonly, readwrite, opencreate };
    enum sqlite_state { sqlite_done, sqlite_row, sqlite_error, sqlite_constraint, sqlite_locked };
    enum uuid_format { uuid_text, uuid_binary };

    class blob;
    class null;

    class ADFSSHARED_EXPORT sqlite {
        sqlite( const sqlite& ) = delete;
        sqlite& operator = ( const sqlite& ) = delete;

        sqlite3 * db_;
        std::function< void( const char *) > error_handler_;
        static uuid_format uuid_format_;
        uint32_t fs_format_version_;
    public:
        ~sqlite();
        sqlite();

        inline operator sqlite3 * () { return db_; }
        bool open( const wchar_t * path );
        bool open( const char * path );
        bool open( const char * path, adfs::flags );
        bool close();
        void error_message( const char * msg );
        void register_error_handler( std::function<void( const char * )> );
        void set_fs_format_version( uint32_t );
        uint32_t fs_format_version() const;        
        static void uuid_storage_format( uuid_format );
        static uuid_format uuid_storage_format();
    };

    class ADFSSHARED_EXPORT blob {
        const int8_t * p_;
        std::size_t octets_;
        sqlite3_blob * pBlob_;
    public:
        ~blob();
        blob();
        blob( std::size_t octets, const int8_t *p = 0 );
        blob( std::size_t octets, const char *p = 0 );
        uint32_t size() const;
        inline const int8_t * data() const { return p_; }
        inline operator bool () const { return pBlob_ != 0; }
        bool close();
        bool open( sqlite& db, const char * zDb, const char * zTable, const char * zColumn, int64_t rowid, flags );
        bool reopen( int64_t rowid );
        bool read( int8_t *, std::size_t, std::size_t offset = 0 ) const;
        bool write( const int8_t *, std::size_t, std::size_t offset = 0 ) const;
    };

    class ADFSSHARED_EXPORT null { };
    class ADFSSHARED_EXPORT error { };

    // typedef boost::variant< long, double, std::wstring, blob, null > column_value_type;

    class ADFSSHARED_EXPORT stmt {
    public:
        ~stmt();
        stmt( sqlite& );

        bool begin();
        bool commit();
        bool rollback();

        bool exec( const std::string& );
        //
        bool prepare( const std::string& );
        bool prepare( const std::wstring& );
        bool reset();
        int errcode() const;
        std::string errmsg() const;

        sqlite_state step();

        //
        bool bind_blob( int, const void *, std::size_t, void(*)(void*) = 0);
        bool bind_zeroblob( int, std::size_t );
        //
        struct ADFSSHARED_EXPORT bind_item {
            sqlite3_stmt * stmt_;
            int nnn_;
            bind_item( sqlite3_stmt * stmt, int nnn ) : stmt_(stmt), nnn_(nnn) {}
            template<typename T> bool operator = ( const T& t );
        };

        bind_item bind( int );
        bind_item bind( const std::string& );

        int column_count() const;
        int column_type( int ) const;
        std::string column_name( int ) const;
        std::wstring wcolumn_name( int ) const;
        std::string column_decltype( int ) const;
        int data_count() const;

        // column_value_type column_value( int );
        template<typename T> T get_column_value( int ) const;
        bool is_null_column( int ) const; // no value assigned or no column does exist
        const char * sql() const;
        std::string expanded_sql() const;

    private:
        sqlite& sqlite_;
        sqlite3_stmt * stmt_;
        bool transaction_active_;
        static int callback( void *, int argc, char ** argv, char ** azColName );
    };

}

