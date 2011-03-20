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

namespace adfs {

    class sqlite : boost::noncopyable {
        sqlite3 * db_;
    public:
        ~sqlite();
        sqlite();
        sqlite( const sqlite& );

        inline operator sqlite3 * () { return db_; }

        bool open( const std::wstring& path );
        bool create( const std::wstring& path );
        bool close();
    };

    class blob {
    public:
        blob();
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
        bool bind( int, double );
        bool bind( int, int );
        bool bind( int, boost::int64_t );
        bool bind( int );
        bool bind( int, const std::string&, void(*)(void*) = 0);
        bool bind( int, const std::wstring&, void(*)(void*) = 0);
        // bool bind_value( int, const sqlite3_value* );
        bool bind_zeroblob( int, std::size_t );
        //
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
