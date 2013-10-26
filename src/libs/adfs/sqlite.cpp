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

#include "sqlite.hpp"
#include "sqlite3.h"
#include <adportable/string.hpp>
#include <adportable/utf.hpp>
#include <adportable/debug.hpp>
#include <iostream>

#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>

namespace adfs {

    struct Msg {
        char * p;
        inline operator char ** () { return &p; }
        Msg() : p(0) {}
        ~Msg() { sqlite3_free( p ); }
    };

    namespace detail {
        struct error_log {
            static void log( const std::string& sql, const char * msg ) {
				adportable::debug(__FILE__, __LINE__) << sql << "\terror : " << msg;
            }
            static void log( const std::wstring& sql, const char * msg ) {
				adportable::debug(__FILE__, __LINE__) << sql << "\terror : " << msg;
            }
        };
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
sqlite::open( const wchar_t * path )
{
    return ::sqlite3_open16( reinterpret_cast< const void *>(path), &db_ ) == SQLITE_OK;
}

bool
sqlite::open( const char * path )
{
    return sqlite3_open( path, &db_ ) == SQLITE_OK;
}

bool
sqlite::close()
{
    sqlite3 * temp = db_;
	if ( sqlite3_close( temp ) == SQLITE_OK ) {
		db_ = 0;
		return true;
	}
	return false;
}

//////////////////////

stmt::~stmt()
{
    if ( transaction_active_ )
        rollback();
    sqlite3_finalize( stmt_ );
}

stmt::stmt( sqlite& db ) : sqlite_(db), stmt_(0), transaction_active_(false)
{
}

bool
stmt::begin()
{
    Msg msg;
    if ( sqlite3_exec( sqlite_, "BEGIN DEFERRED", callback, 0, msg ) == SQLITE_OK ) {
        transaction_active_ = true;
        return true;
    }
    detail::error_log::log( "BEGIN DEFERRED", msg.p );
    return false;
}

bool
stmt::commit()
{
    Msg msg;
    transaction_active_ = false;
    if ( sqlite3_exec( sqlite_, "COMMIT", callback, 0, msg ) == SQLITE_OK )
        return true;
    detail::error_log::log( "COMMIT", msg.p );
    return false;
}

bool
stmt::rollback()
{
    Msg msg;
    transaction_active_ = false;
    if ( sqlite3_exec( sqlite_, "ROLLBACK", callback, 0, msg ) == SQLITE_OK )
        return true;
    detail::error_log::log( "ROOLBACK", msg.p );
    return false;
}

bool
stmt::exec( const std::string& sql )
{
    Msg msg;
    if ( sqlite3_exec( sqlite_, sql.c_str(), callback, 0, msg ) == SQLITE_OK )
        return true;
    detail::error_log::log( sql, msg.p );
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
    detail::error_log::log( sql, sqlite3_errmsg( sqlite_ ) );
    return false;
}

bool
stmt::prepare( const std::wstring& sql )
{
    const wchar_t * tail = 0;
    if ( sqlite3_prepare16_v2( sqlite_, sql.c_str(), -1, &stmt_, reinterpret_cast< const void ** >(&tail) ) == SQLITE_OK )
        return true;
    detail::error_log::log( sql, sqlite3_errmsg( sqlite_ ) );
    return false;
}

bool
stmt::reset()
{
    return sqlite3_reset( stmt_ ) == SQLITE_OK;
}

int
stmt::errcode()
{
    return sqlite3_errcode( sqlite_ );
}

sqlite_state
stmt::step()
{
    int rc = sqlite3_step( stmt_ );
    switch( rc ) {
    case SQLITE_ROW:   return sqlite_row;
    case SQLITE_DONE:  return sqlite_done;
    case SQLITE_CONSTRAINT: return sqlite_constraint;
    default: break;
    }
    detail::error_log::log( "", sqlite3_errmsg( sqlite_ ) );
    return sqlite_error;
}

bool
stmt::bind_blob( int nnn, const void * blob, std::size_t size, void dtor(void*) )
{
    return sqlite3_bind_blob( stmt_, nnn, blob, size, dtor ) == SQLITE_OK;
}

bool
stmt::bind_zeroblob( int nnn, std::size_t size )
{
    return sqlite3_bind_zeroblob( stmt_, nnn, size ) == SQLITE_OK;
}

//-------------
stmt::bind_item
stmt::bind( int nnn )
{
    return bind_item( stmt_, nnn );
}

stmt::bind_item
stmt::bind( const std::string& column )
{
    for ( int i = 0; i < sqlite3_column_count( stmt_ ); ++i ) {
        if ( sqlite3_column_name( stmt_, i ) == column ) {
            return bind_item( stmt_, i + 1 );
        }
    }
    return bind_item( 0, 0 );
}

namespace adfs {

    template<> bool
    stmt::bind_item::operator = ( const int32_t& v )
    {
        return sqlite3_bind_int( stmt_, nnn_, v ) == SQLITE_OK;
    }
    
    template<> bool 
    stmt::bind_item::operator = ( const boost::uint32_t& v )
    {
        return sqlite3_bind_int( stmt_, nnn_, v ) == SQLITE_OK;
    }
    
    template<> bool
    stmt::bind_item::operator = ( const long & v )
    {
        return sqlite3_bind_int( stmt_, nnn_, v ) == SQLITE_OK;
    }
    
    template<> bool 
    stmt::bind_item::operator = ( const unsigned long & v )
    {
        return sqlite3_bind_int( stmt_, nnn_, v ) == SQLITE_OK;
    }
    
    template<> bool
    stmt::bind_item::operator = ( const int64_t& v )
    {
        return sqlite3_bind_int64( stmt_, nnn_, v ) == SQLITE_OK;
    }

	template<> bool
    stmt::bind_item::operator = ( const double& v )
    {
        return sqlite3_bind_double( stmt_, nnn_, v ) == SQLITE_OK;
    }
    
    template<> bool
    stmt::bind_item::operator = ( const boost::uint64_t& v )
    {
        return sqlite3_bind_int64( stmt_, nnn_, v ) == SQLITE_OK;
    }
    
    template<> bool
    stmt::bind_item::operator = ( const std::string& v )
    {
        return sqlite3_bind_text( stmt_, nnn_, v.c_str(), -1, SQLITE_TRANSIENT ) == SQLITE_OK;
    }
    
    template<> bool
    stmt::bind_item::operator = ( const std::wstring& v )
    {
        std::string u = adportable::utf::to_utf8( v );
        return sqlite3_bind_text( stmt_, nnn_, reinterpret_cast<const char *>( u.c_str() )
                                  , -1, SQLITE_TRANSIENT ) == SQLITE_OK;
    }

    template<> bool
    stmt::bind_item::operator = ( const blob& blob )
    {
        if ( blob.get() )
            return sqlite3_bind_blob( stmt_, nnn_, blob.get(), blob.size(), 0 ) == SQLITE_OK;
        else
            return sqlite3_bind_zeroblob( stmt_, nnn_, blob.size() ) == SQLITE_OK;
    }

    template<> std::string stmt::get_column_value( int nCol )
    {
        if ( sqlite3_column_type( stmt_, nCol ) == SQLITE_TEXT ) {
            const unsigned char * text = sqlite3_column_text( stmt_, nCol );
            return std::string( reinterpret_cast< const char * >(text) );
        }
        throw std::bad_cast();
    }

    template<> std::wstring stmt::get_column_value( int nCol )
    {
        if ( sqlite3_column_type( stmt_, nCol ) == SQLITE_TEXT ) {
            const unsigned char * text = sqlite3_column_text( stmt_, nCol );
            return adportable::utf::to_wstring( text );
        }
        throw std::bad_cast();
    }

    template<> double stmt::get_column_value( int nCol )
    {
        if ( sqlite3_column_type( stmt_, nCol ) == SQLITE_FLOAT )
            return sqlite3_column_double( stmt_, nCol );
        throw std::bad_cast();
    }

    template<> uint64_t stmt::get_column_value( int nCol )
    {
        if ( sqlite3_column_type( stmt_, nCol ) == SQLITE_INTEGER )
            return sqlite3_column_int64( stmt_, nCol );
        throw std::bad_cast();
    }

    template<> blob stmt::get_column_value( int nCol )
    {
        if ( sqlite3_column_type( stmt_, nCol ) == SQLITE_BLOB )
            return blob();
        throw std::bad_cast();
    }

} /* namespace adfs */

//-------------
int
stmt::column_count()
{
    return sqlite3_column_count( stmt_ );
}

int
stmt::column_type( int nCol )
{
    std::string decl = sqlite3_column_decltype( stmt_, nCol );
    (void)decl;
    return sqlite3_column_type( stmt_, nCol );
}

column_value_type
stmt::column_value( int nCol )
{
    switch( sqlite3_column_type( stmt_, nCol ) ) {
    case SQLITE_INTEGER: return column_value_type( sqlite3_column_int64( stmt_, nCol ) );
    case SQLITE_FLOAT:   return column_value_type( sqlite3_column_double( stmt_, nCol ) );
    case SQLITE_TEXT:    
        do {
            const unsigned char * text = sqlite3_column_text( stmt_, nCol );
			return column_value_type( adportable::utf::to_wstring( text ) );
        } while(0);
    case SQLITE_BLOB:    return column_value_type( blob() );
    case SQLITE_NULL:    return column_value_type( null() );
    default: break;
    };
    return column_value_type( null() );
}

///////////////////

blob::~blob()
{
    close();
}

blob::blob() : p_(0), octets_(0), pBlob_(0)
{
}

blob::blob( std::size_t octets, const int8_t *p ) : p_(p), octets_( octets ), pBlob_(0)
{
}

boost::uint32_t
blob::size() const
{ 
    return octets_;
}

bool
blob::close()
{
    if ( pBlob_ ) {
        sqlite3_blob_close( pBlob_ );
        pBlob_ = 0;
    }
    return true;
}

bool
blob::open( sqlite& db, const char * zDb, const char * zTable, const char * zColumn, int64_t rowid, flags flag )
{
    if ( pBlob_ )
        close();

    if ( sqlite3_blob_open( db, zDb, zTable, zColumn, rowid, static_cast<int>(flag), &pBlob_ ) == SQLITE_OK ) {
        octets_ = sqlite3_blob_bytes( pBlob_ );
        return true;
    }
    return false;
}

bool
blob::reopen( int64_t rowid )
{
    if ( pBlob_ && ( sqlite3_blob_reopen( pBlob_, rowid ) == SQLITE_OK ) ) {
        octets_ = sqlite3_blob_bytes( pBlob_ );
        return true;
    }
    return false;
}

bool
blob::read( int8_t * pbuf, std::size_t bufsize, std::size_t offset ) const
{
    if ( pBlob_ && ( sqlite3_blob_read( pBlob_, pbuf, bufsize, offset ) == SQLITE_OK ) )
        return true;
    return false;
}

bool
blob::write( const int8_t * pbuf, std::size_t octets, std::size_t offset ) const
{
    return sqlite3_blob_write( pBlob_, pbuf, octets, offset ) == SQLITE_OK;
}

///////////////////
