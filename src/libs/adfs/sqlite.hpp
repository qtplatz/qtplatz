// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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

#pragma once

#include <boost/noncopyable.hpp>
#include <boost/cstdint.hpp>
#include <compiler/workaround.h>
#include <boost/variant.hpp>
#include <string>

struct sqlite3;
struct sqlite3_stmt;
struct sqlite3_blob;

namespace adfs {

    enum flags { readonly, readwrite };
    enum sqlite_state { sqlite_done, sqlite_row, sqlite_error, sqlite_constraint };

    class blob;
    class null;

    class sqlite : boost::noncopyable {
        sqlite3 * db_;
    public:
        ~sqlite();
        sqlite();
        sqlite( const sqlite& );

        inline operator sqlite3 * () { return db_; }
        bool open( const wchar_t * path );
        bool open( const char * path );
        bool close();
    };

    class blob {
        const boost::int8_t * p_;
        std::size_t octets_;
        sqlite3_blob * pBlob_;
    public:
        ~blob();
        blob();
        blob( std::size_t octets, const boost::int8_t *p = 0 );
        boost::uint32_t size() const;
        inline const boost::int8_t * get() const { return p_; }
        inline operator bool () const { return pBlob_ != 0; }
        bool close();
        bool open( sqlite& db, const char * zDb, const char * zTable, const char * zColumn, boost::int64_t rowid, flags );
        bool reopen( boost::int64_t rowid );
        bool read( boost::int8_t *, std::size_t, std::size_t offset = 0 ) const;
        bool write( const boost::int8_t *, std::size_t, std::size_t offset = 0 ) const;
    };

    class null { };
    class error { };

    typedef boost::variant< boost::int64_t, double, std::wstring, blob, null > column_value_type;

    class stmt {
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
        int errcode();

        sqlite_state step();

        //
        bool bind_blob( int, const void *, std::size_t, void(*)(void*) = 0);
        bool bind_zeroblob( int, std::size_t );
        //
        struct bind_item {
            sqlite3_stmt * stmt_;
            int nnn_;
            bind_item( sqlite3_stmt * stmt, int nnn ) : stmt_(stmt), nnn_(nnn) {}
            template<typename T> bool operator = ( const T& t ); // { stmt_.bind( nnn_, t ); }
        };
        bind_item bind( int );
        bind_item bind( const std::string& );

        int column_count();
        int column_type( int );

        column_value_type column_value( int );

    private:
        sqlite& sqlite_;
        sqlite3_stmt * stmt_;
        bool transaction_active_;
        static int callback( void *, int argc, char ** argv, char ** azColName );
    };

}

