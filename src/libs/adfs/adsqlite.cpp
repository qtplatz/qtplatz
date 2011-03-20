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

#include "adsqlite.h"
#include "sqlite3.h"
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>

namespace adfs { // namespace detail {
/*
    class sqlite : boost::noncopyable {
        ::sqlite3 * db_;
    public:
        ~sqlite();
        sqlite();
        bool open( const std::wstring& path );
    };
*/
    struct Msg {
        char * p;
        inline operator char ** () { return &p; }
        Msg() : p(0) {}
        ~Msg() { sqlite3_free( p ); }
    };

} // adfs

using namespace adfs;

sqlite::~sqlite()
{ 
    if ( db_ )
        sqlite3_close( db_ );
}

sqlite::sqlite() : db_(0)
{
}

bool
sqlite::open( const std::wstring& path )
{
    return sqlite3_open16( path.c_str(), &db_ ) == SQLITE_OK;
}

//////////////////////

stmt::~stmt()
{
    sqlite3_finalize( stmt_ );
}

stmt::stmt( sqlite& db ) : sqlite_(db), stmt_(0)
{
}

bool
stmt::begin()
{
    Msg msg;
    if ( sqlite3_exec( sqlite_, "BEGIN DEFERRED", callback, 0, msg ) == SQLITE_OK )
        return true;
    std::cout << msg.p << std::endl;
    return false;
}

bool
stmt::commit()
{
    Msg msg;
    if ( sqlite3_exec( sqlite_, "COMMIT", callback, 0, msg ) == SQLITE_OK )
        return true;
    std::cout << msg.p << std::endl;
    return false;
}

bool
stmt::rollback()
{
    Msg msg;
    if ( sqlite3_exec( sqlite_, "ROLLBACK", callback, 0, msg ) == SQLITE_OK )
        return true;
    std::cout << msg.p << std::endl;
    return false;
}

bool
stmt::exec( const std::string& sql )
{
    Msg msg;
    if ( sqlite3_exec( sqlite_, sql.c_str(), callback, 0, msg ) == SQLITE_OK )
        return true;
    std::cout << msg.p << std::endl;
    return false;
}

int
stmt::callback( void *, int argc, char ** argv, char ** azColName )
{
    (void)argc;
    (void)argv;
    (void)azColName;
    return 0;
}

bool
stmt::prepare( const std::string& sql )
{
    const char * tail = 0;
    if ( sqlite3_prepare_v2( sqlite_, sql.c_str(), -1, &stmt_, &tail ) == SQLITE_OK )
        return true;
    std::cout << sqlite3_errmsg( sqlite_ );
    return false;
}

bool
stmt::prepare( const std::wstring& sql )
{
    const wchar_t * tail = 0;
    if ( sqlite3_prepare16_v2( sqlite_, sql.c_str(), -1, &stmt_, reinterpret_cast< const void ** >(&tail) ) == SQLITE_OK )
        return true;
    std::cout << sqlite3_errmsg( sqlite_ );
    return false;
}

bool
stmt::reset()
{
    return sqlite3_reset( stmt_ ) == SQLITE_OK;
}

step_state
stmt::step()
{
    int rc = sqlite3_step( stmt_ );
    switch( rc ) {
    case SQLITE_ROW:   return sqlite_row;
    case SQLITE_DONE:  return sqlite_done;
    default: break;
    }
    return sqlite_error;
}

bool
stmt::bind_blob( int nnn, const void * blob, std::size_t size, void dtor(void*) )
{
    return sqlite3_bind_blob( stmt_, nnn, blob, size, dtor ) == SQLITE_OK;
}

bool
stmt::bind( int nnn, double value )
{
    return sqlite3_bind_double( stmt_, nnn, value ) == SQLITE_OK;
}

bool
stmt::bind( int nnn, int value )
{
    return sqlite3_bind_int64( stmt_, nnn, value ) == SQLITE_OK;
}

bool
stmt::bind( int nnn, boost::int64_t value )
{
    return sqlite3_bind_int64( stmt_, nnn, value ) == SQLITE_OK;
}

bool
stmt::bind( int nnn )
{
    return sqlite3_bind_null( stmt_, nnn ) == SQLITE_OK;
}

bool
stmt::bind( int nnn, const std::string& value, void dtor(void*) )
{
    return sqlite3_bind_text( stmt_, nnn, value.c_str(), value.length(), dtor ) == SQLITE_OK;
}

bool
stmt::bind( int nnn, const std::wstring& value , void dtor(void*) )
{
    return sqlite3_bind_text16( stmt_, nnn, value.c_str(), value.length(), dtor ) == SQLITE_OK;
}

// bool bind_value( int, const sqlite3_value* );
bool
stmt::bind_zeroblob( int nnn, std::size_t size )
{
    return sqlite3_bind_zeroblob( stmt_, nnn, size ) == SQLITE_OK;
}

int
stmt::column_count()
{
    return sqlite3_column_count( stmt_ );
}

int
stmt::column_type( int nCol )
{
/*
#define SQLITE_INTEGER  1
#define SQLITE_FLOAT    2
#define SQLITE_TEXT     3
#define SQLITE_BLOB     4
#define SQLITE_NULL     5
*/
    return sqlite3_column_type( stmt_, nCol );
}

result_value_type
stmt::column_value( int nCol )
{
    switch( sqlite3_column_type( stmt_, nCol ) ) {
    case SQLITE_INTEGER: return result_value_type( sqlite3_column_int( stmt_, nCol ) );
    case SQLITE_FLOAT:   return result_value_type( sqlite3_column_double( stmt_, nCol ) );
    case SQLITE_TEXT:    return result_value_type( reinterpret_cast<const wchar_t*>(sqlite3_column_text16( stmt_, nCol )) );
    case SQLITE_BLOB:    return result_value_type( blob() );
    case SQLITE_NULL:    return result_value_type( null() );
    default: break;
    };
    return result_value_type( null() );
}

///////////////////
blob::blob()
{
}

///////////////////
