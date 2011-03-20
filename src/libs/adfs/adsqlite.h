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
        bool step();

        //
        bool bind_blob( int, const void *, std::size_t, void(*)(void*) = 0);
        bool bind_double( int, double );
        bool bind_int( int, int );
        bool bind_int64( int, long long );
        bool bind_null( int );
        bool bind_text( int, const std::string&, void(*)(void*) = 0);
        bool bind_text( int, const std::wstring&, void(*)(void*) = 0);
        // bool bind_value( int, const sqlite3_value* );
        bool bind_zeroblob( int, std::size_t );
        //

    private:
        sqlite& sqlite_;
        sqlite3_stmt * stmt_;
        static int callback( void *, int argc, char ** argv, char ** azColName );
    };

}

#endif // ADSQLITE_H
