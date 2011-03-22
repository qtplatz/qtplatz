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

#ifndef ADSQLITE_H
#define ADSQLITE_H

#include <boost/noncopyable.hpp>
#include <boost/cstdint.hpp>
#include <boost/variant.hpp>
#include <string>

struct sqlite3;
struct sqlite3_stmt;
struct sqlite3_blob;

namespace adfs {

    enum flags { readonly, readwrite };

    class sqlite : boost::noncopyable {
        sqlite3 * db_;
    public:
        ~sqlite();
        sqlite();
        sqlite( const sqlite& );

        inline operator sqlite3 * () { return db_; }

        bool open( const std::wstring& path );
        // bool create( const std::wstring& path );
        bool close();
    };

    class blob {
        const boost::uint8_t * p_;
        std::size_t octets_;
        sqlite3_blob * pBlob_;
    public:
        ~blob();
        blob();
        blob( std::size_t octets, const boost::uint8_t *p = 0 );
        boost::uint32_t size() const;
        inline const boost::uint8_t * get() const { return p_; }
        inline operator bool () const { return pBlob_ != 0; }
        bool close();
        bool open( sqlite& db, const char * zDb, const char * zTable, const char * zColumn, boost::int64_t rowid, flags );
        bool reopen( boost::int64_t rowid );
        bool read( boost::int8_t *, std::size_t, std::size_t offset = 0 ) const;
        bool write( const boost::int8_t *, std::size_t, std::size_t offset = 0 ) const;
    };

    class null { };
    class error { };

    enum step_state { sqlite_done, sqlite_row, sqlite_error };

    typedef boost::variant< int, boost::int64_t, double, std::string, std::wstring, blob, null, error > result_value_type;

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

        step_state step();

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

        result_value_type column_value( int );

    private:
        sqlite& sqlite_;
        sqlite3_stmt * stmt_;
        static int callback( void *, int argc, char ** argv, char ** azColName );
    };

}

#endif // ADSQLITE_H
