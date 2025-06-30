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

#include "adfs.hpp"
#include "sqlite.hpp"
#include "sqlite3.h"
#if LOG_ADLOG
#include <adlog/logger.hpp>
#else
#include <adportable/debug.hpp>
#endif
#include <adportable/string.hpp>
#include <adportable/utf.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <locale>

namespace adfs {

    struct bad_cast : virtual boost::exception, virtual std::bad_cast { };
    typedef boost::error_info< struct tage_errmsg, std::string > error_info;

    struct Msg {
        char * p;
        inline operator char ** () { return &p; }
        Msg() : p(0) {}
        ~Msg() { sqlite3_free( p ); }
    };

    namespace {
        struct error_log {
            template< typename T >
            static void log( const /* std::string& */ T& sql, const char * msg, const char * file, int line ) {
#if LOG_ADLOG
                adlog::logger( file, line ) << sql << "\terror : " << (msg ? msg : "nullstr");
#else
                adportable::debug( file, line ) << sql << "\terror : " << (msg ? msg : "nullstr");
#endif
            }

            std::string operator()( std::tuple< int, std::string, std::string, std::string > t, const char * __file, int __line ) {
                std::ostringstream o;
                int rcode;
                std::string emsg, file, sql;
                std::tie( rcode, emsg, file, sql ) = t;
                o << "sqlite rcode:\t" << rcode << "\t" << emsg;
                if ( ! file.empty() )
                    o << "\n\t--\tfile:\t" << file;
                if ( ! sql.empty() )
                    o << "\n\t--\tsql:\t" << sql;
#if LOG_ADLOG
                adlog::logger( __file, __line ) << o.str();
#else
                adportable::debug( __file, __line ) << "***** " << o.str();
#endif
                return o.str();
            }

        };
    };

    template<class Facet>
    struct deletable_facet : Facet {
        template<class ...Args>
        deletable_facet( Args&& ...args) : Facet(std::forward<Args>(args)...) {}
        ~deletable_facet() {}
    };

    class sqlite_exception : public boost::exception, public std::exception {};

    uuid_format sqlite::uuid_format_ = uuid_text;
} // adfs

using namespace adfs;

//static
void
sqlite::uuid_storage_format( uuid_format fmt )
{
    uuid_format_ = fmt;
}

//static
uuid_format
sqlite::uuid_storage_format()
{
    return uuid_format_;
}

sqlite::~sqlite()
{
    if ( db_ )
        sqlite3_close( db_ );
}

sqlite::sqlite() : db_( 0 )
                 , fs_format_version_( 0 )
{
}

void
sqlite::set_fs_format_version( uint32_t v )
{
    fs_format_version_ = v;
}

uint32_t
sqlite::fs_format_version() const
{
    return fs_format_version_;
}

// void
// sqlite::register_error_handler( std::function< void(const char * )> f )
// {
//     error_handler_ = f;
// }

// void
// sqlite::error_message( const char * msg )
// {
//     if ( error_handler_ )
//         error_handler_( msg );
// }

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
sqlite::open( const char * path, adfs::flags flags )
{
    int mode = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;

    if ( flags == adfs::readonly )
        mode = SQLITE_OPEN_READONLY;
    else if ( flags == adfs::readwrite )
        mode = SQLITE_OPEN_READWRITE;
    else if ( flags == adfs::opencreate )
        mode = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;

    return sqlite3_open_v2( path, &db_, mode, 0 ) == SQLITE_OK;
}

bool
sqlite::close()
{
    sqlite3 * temp = db_;
    int errc( 0 );
	if ( ( errc = sqlite3_close( temp ) ) == SQLITE_OK ) {
		db_ = 0;
		return true;
	}
	return false;
}

int64_t
sqlite::last_insert_rowid() const
{
    return sqlite3_last_insert_rowid( db_ );
}

std::string
sqlite::db_filename( const char * zDbName )
{
    return ::sqlite3_db_filename( db_, zDbName );
}

//////////////////////

stmt::~stmt()
{
    if ( transaction_active_ )
        commit();
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
    //sqlite_.error_message( msg.p );
    error_log::log( "BEGIN DEFERRED", msg.p, __FILE__, __LINE__ );
    return false;
}

bool
stmt::commit()
{
    Msg msg;
    transaction_active_ = false;
    if ( sqlite3_exec( sqlite_, "COMMIT", callback, 0, msg ) == SQLITE_OK )
        return true;
    //sqlite_.error_message( msg.p );
    error_log::log( "COMMIT", msg.p, __FILE__, __LINE__ );
    return false;
}

bool
stmt::rollback()
{
    Msg msg;
    transaction_active_ = false;
    if ( sqlite3_exec( sqlite_, "ROLLBACK", callback, 0, msg ) == SQLITE_OK )
        return true;
    //sqlite_.error_message( msg.p );
    error_log::log( "ROOLBACK", msg.p, __FILE__, __LINE__ );
    return false;
}

bool
stmt::exec( const std::string& sql )
{
    Msg msg;
    if ( sqlite3_exec( sqlite_, sql.c_str(), callback, 0, msg ) == SQLITE_OK )
        return true;
    //sqlite_.error_message( msg.p );
    error_log::log( sql, msg.p, __FILE__, __LINE__ );
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
    if ( stmt_ )
        sqlite3_finalize( stmt_ );

    const char * tail = 0;
    int rcode;
    if ( (rcode = sqlite3_prepare_v2( sqlite_, sql.c_str(), -1, &stmt_, &tail )) == SQLITE_OK )
        return true;

    if ( rcode == SQLITE_ERROR ) // no such table
        return false;

    error_log()( error_details( rcode ), __FILE__, __LINE__ );
    return false;
}

bool
stmt::prepare( const std::wstring& sql )
{
    if ( stmt_ )
        sqlite3_finalize( stmt_ );

    auto utf8 = adportable::utf::to_utf8( sql );

    const char * tail = 0;
    if ( sqlite3_prepare_v2( sqlite_, utf8.c_str(), -1, &stmt_, &tail ) == SQLITE_OK )
        return true;
    error_log::log( sql, sqlite3_errmsg( sqlite_ ), __FILE__, __LINE__ );
    return false;
}

bool
stmt::reset()
{
    return sqlite3_reset( stmt_ ) == SQLITE_OK;
}

int
stmt::errcode() const
{
    return sqlite3_errcode( sqlite_ );
}

int
stmt::extended_errcode() const
{
    return sqlite3_extended_errcode( sqlite_ );
}

std::string
stmt::errmsg() const
{
    return std::string( sqlite3_errmsg( sqlite_ ) );
}

sqlite_state
stmt::step()
{
    int rc = sqlite3_step( stmt_ );
    switch( rc ) {
    case SQLITE_OK:    return sqlite_ok;
    case SQLITE_ROW:   return sqlite_row;
    case SQLITE_DONE:  return sqlite_done;
    case SQLITE_CONSTRAINT: return sqlite_constraint;
    case SQLITE_LOCKED: return sqlite_locked;
    case SQLITE_MISUSE: return sqlite_misuse; // not an error
    default: break;
    }

    error_log()( error_details( rc ), __FILE__, __LINE__ );

    return sqlite_error;
}

bool
stmt::bind_blob( int nnn, const void * blob, std::size_t size, void dtor(void*) )
{
    return sqlite3_bind_blob( stmt_, nnn, blob, static_cast<int>(size), dtor ) == SQLITE_OK;
}

bool
stmt::bind_zeroblob( int nnn, std::size_t size )
{
    return sqlite3_bind_zeroblob( stmt_, nnn, static_cast<int>(size) ) == SQLITE_OK;
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

bool
stmt::is_null_column( int nCol ) const
{
    return sqlite3_column_type( stmt_, nCol ) == SQLITE_NULL;
}

int
stmt::data_count() const
{
    return sqlite3_data_count( stmt_ );
}

const char *
stmt::sql() const
{
    return sqlite3_sql( stmt_ );
}

std::string
stmt::expanded_sql() const
{
    std::string sql;
    if ( char * p = sqlite3_expanded_sql( stmt_ ) ) {
        sql = p;
        sqlite3_free( p );
    }
    return sql;
}

std::tuple< int, std::string, std::string, std::string >
stmt::error_details( int rcode ) const
{
    std::string emsg, query, file;

    int errc = sqlite3_errcode( sqlite_ );    (void)errc;
    assert( rcode == errc );

    if ( auto p = sqlite3_sql( stmt_ ) )
        query = std::string( p );

    if ( auto p = sqlite3_errmsg( sqlite_ ) )
        emsg = std::string( p );

    sqlite3_stmt * stmt(0);
    const char * tail(0);
    if ( sqlite3_prepare_v2( sqlite_, "SELECT file FROM pragma_database_list WHERE name='main'", -1, &stmt, &tail ) == SQLITE_OK ) {
        if ( sqlite3_step( stmt ) == SQLITE_ROW )
            file = reinterpret_cast< const char * >( sqlite3_column_text( stmt, 0 ) );
    }
    sqlite3_finalize( stmt );

    return std::make_tuple( rcode, emsg, file, query );
}

namespace adfs {

    template<> ADFSSHARED_EXPORT bool
    stmt::bind_item::operator = ( const int32_t& v )
    {
        return sqlite3_bind_int( stmt_, nnn_, v ) == SQLITE_OK;
    }

    template<> ADFSSHARED_EXPORT bool
    stmt::bind_item::operator = ( const uint32_t& v )
    {
        return sqlite3_bind_int( stmt_, nnn_, v ) == SQLITE_OK;
    }

    template<> ADFSSHARED_EXPORT bool
    stmt::bind_item::operator = ( const long & v )
    {
#if _MSC_VER
        return sqlite3_bind_int( stmt_, nnn_, v ) == SQLITE_OK;
#else
        return sqlite3_bind_int64( stmt_, nnn_, v ) == SQLITE_OK;
#endif
    }

    template<> ADFSSHARED_EXPORT bool
    stmt::bind_item::operator = ( const unsigned long & v )
    {
#if _MSC_VER
        return sqlite3_bind_int( stmt_, nnn_, v ) == SQLITE_OK;
#else
        return sqlite3_bind_int64( stmt_, nnn_, v ) == SQLITE_OK;
#endif
    }

#if (defined __x86_64__ && defined __linux__ )
    // int64_t, long and long long on linux gcc x86_64 is identical (8 byte int)
    template<> bool
    stmt::bind_item::operator = ( const long long & v )
    {
        return sqlite3_bind_int64( stmt_, nnn_, v ) == SQLITE_OK;
    }
    template<> bool
    stmt::bind_item::operator = ( const unsigned long long & v )
    {
        return sqlite3_bind_int64( stmt_, nnn_, v ) == SQLITE_OK;
    }
#else
    template<> ADFSSHARED_EXPORT bool
    stmt::bind_item::operator = ( const int64_t& v )
    {
        return sqlite3_bind_int64( stmt_, nnn_, v ) == SQLITE_OK;
    }
    template<> ADFSSHARED_EXPORT bool
    stmt::bind_item::operator = ( const uint64_t& v )
    {
        return sqlite3_bind_int64( stmt_, nnn_, v ) == SQLITE_OK;
    }
#endif
    template<> ADFSSHARED_EXPORT bool
    stmt::bind_item::operator = ( const double& v )
    {
        return sqlite3_bind_double( stmt_, nnn_, v ) == SQLITE_OK;
    }

    template<> ADFSSHARED_EXPORT bool
    stmt::bind_item::operator = ( const std::string& v )
    {
        return sqlite3_bind_text( stmt_, nnn_, v.c_str(), -1, SQLITE_TRANSIENT ) == SQLITE_OK;
    }

    template<> ADFSSHARED_EXPORT bool
    stmt::bind_item::operator = ( const std::wstring& v )
    {
        std::string u = adportable::utf::to_utf8( v );
        return sqlite3_bind_text( stmt_, nnn_, reinterpret_cast<const char *>( u.c_str() )
                                  , -1, SQLITE_TRANSIENT ) == SQLITE_OK;
    }

    template<> ADFSSHARED_EXPORT bool
    stmt::bind_item::operator = ( const blob& blob )
    {
        if ( blob.data() )
            return sqlite3_bind_blob( stmt_, nnn_, blob.data(), blob.size(), 0 ) == SQLITE_OK;
        else
            return sqlite3_bind_zeroblob( stmt_, nnn_, blob.size() ) == SQLITE_OK;
    }

    template<> ADFSSHARED_EXPORT bool
    stmt::bind_item::operator = ( const null& )
    {
        return sqlite3_bind_null( stmt_, nnn_ ) == SQLITE_OK;
    }

    template<> ADFSSHARED_EXPORT bool
    stmt::bind_item::operator = ( const boost::uuids::uuid& uuid )
    {
        if ( sqlite::uuid_storage_format() == uuid_binary )
            return sqlite3_bind_blob( stmt_, nnn_, uuid.begin(), int( uuid.size() ), [] ( void * )->void{} ) == SQLITE_OK;
        else
            return sqlite3_bind_text( stmt_, nnn_, boost::lexical_cast<std::string>(uuid).c_str(), -1, SQLITE_TRANSIENT ) == SQLITE_OK;
    }

    //------------------------------------

    template<> ADFSSHARED_EXPORT std::string stmt::get_column_value( int nCol ) const
    {
        if ( sqlite3_column_type( stmt_, nCol ) == SQLITE_TEXT ) {
            const unsigned char * text = sqlite3_column_text( stmt_, nCol );
            return std::string( reinterpret_cast< const char * >(text) );
        } else if ( sqlite3_column_type( stmt_, nCol ) == SQLITE_NULL )
            return "";

        BOOST_THROW_EXCEPTION( adfs::bad_cast() <<
                               error_info( ( boost::format("attempt to get string from %1% but was %2%")
                                             % column_name( nCol )  % column_decltype( nCol ) ).str() ) );
    }

    template<> ADFSSHARED_EXPORT std::wstring stmt::get_column_value( int nCol ) const
    {
        if ( sqlite3_column_type( stmt_, nCol ) == SQLITE_TEXT ) {
            const unsigned char * text = sqlite3_column_text( stmt_, nCol );
            return adportable::utf::to_wstring( text );
        } else if ( sqlite3_column_type( stmt_, nCol ) == SQLITE_NULL )
            return L"";

        BOOST_THROW_EXCEPTION( adfs::bad_cast() <<
                               error_info( ( boost::format("attempt to get string from %1% but was %2%")
                                             % column_name( nCol )  % column_decltype( nCol ) ).str() ) );
    }

    template<> ADFSSHARED_EXPORT double stmt::get_column_value( int nCol ) const
    {
        if ( sqlite3_column_type( stmt_, nCol ) == SQLITE_FLOAT )
            return sqlite3_column_double( stmt_, nCol );
        else if ( sqlite3_column_type( stmt_, nCol ) == SQLITE_NULL )
            return 0;

        BOOST_THROW_EXCEPTION( adfs::bad_cast() <<
                               error_info( ( boost::format("attempt to get string from %1% but was %2%")
                                             % column_name( nCol )  % column_decltype( nCol ) ).str() ) );
    }

    template<> ADFSSHARED_EXPORT int64_t stmt::get_column_value( int nCol ) const
    {
        if ( sqlite3_column_type( stmt_, nCol ) == SQLITE_INTEGER )
            return sqlite3_column_int64( stmt_, nCol );
        else if ( sqlite3_column_type( stmt_, nCol ) == SQLITE_NULL )
            return 0;

        BOOST_THROW_EXCEPTION( adfs::bad_cast() <<
                               error_info( ( boost::format("attempt to get string from %1% but was %2%")
                                             % column_name( nCol )  % column_decltype( nCol ) ).str() ) );
    }

    template<> ADFSSHARED_EXPORT uint64_t stmt::get_column_value( int nCol ) const
    {
        if ( sqlite3_column_type( stmt_, nCol ) == SQLITE_INTEGER )
            return sqlite3_column_int64( stmt_, nCol );
        else if ( sqlite3_column_type( stmt_, nCol ) == SQLITE_NULL )
            return 0;

        BOOST_THROW_EXCEPTION( adfs::bad_cast() <<
                               error_info( ( boost::format("attempt to get string from %1% but was %2%")
                                             % column_name( nCol )  % column_decltype( nCol ) ).str() ) );
    }

    template<> ADFSSHARED_EXPORT blob stmt::get_column_value( int nCol ) const
    {
        if ( sqlite3_column_type( stmt_, nCol ) == SQLITE_BLOB ) {
            const void * pvoid = sqlite3_column_blob( stmt_, nCol );
            int octets = sqlite3_column_bytes( stmt_, nCol );
            return blob( octets, reinterpret_cast<const int8_t *>(pvoid) );
        } else if ( sqlite3_column_type( stmt_, nCol ) == SQLITE_NULL )
            return blob();

        BOOST_THROW_EXCEPTION( adfs::bad_cast() <<
                               error_info( ( boost::format("attempt to get string from %1% but was %2%")
                                             % column_name( nCol )  % column_decltype( nCol ) ).str() ) );
    }

    template<> ADFSSHARED_EXPORT boost::uuids::uuid stmt::get_column_value( int nCol ) const
    {
        if ( sqlite3_column_type( stmt_, nCol ) == SQLITE_TEXT ) {

            const unsigned char * text = sqlite3_column_text( stmt_, nCol );
            return boost::lexical_cast<boost::uuids::uuid>( text );

        } else if ( sqlite3_column_type( stmt_, nCol ) == SQLITE_BLOB ) {
            boost::uuids::uuid uuid;
            auto pBlob = sqlite3_column_blob( stmt_, nCol );
            int octets = std::min( sqlite3_column_bytes( stmt_, nCol ), int( uuid.size() ) );
            std::copy( static_cast<const char *>( pBlob ), static_cast<const char *>(pBlob)+octets, reinterpret_cast<char *>( uuid.begin() ) );
            return uuid;
        } else if ( sqlite3_column_type( stmt_, nCol ) == SQLITE_NULL )
            return boost::uuids::uuid();

        BOOST_THROW_EXCEPTION( adfs::bad_cast() <<
                               error_info( ( boost::format("attempt to get string from %1% but was %2%")
                                             % column_name( nCol )  % column_decltype( nCol ) ).str() ) );
    }

} /* namespace adfs */

//-------------
int
stmt::column_count() const
{
    return sqlite3_column_count( stmt_ );
}

int
stmt::column_type( int nCol ) const
{
    return sqlite3_column_type( stmt_, nCol );
}

std::string
stmt::column_name( int nCol ) const
{
    return sqlite3_column_name( stmt_, nCol );
}

std::string
stmt::column_decltype( int nCol ) const
{
    return null_safe( sqlite3_column_decltype( stmt_, nCol ) );
}

///////////////////

blob::~blob()
{
    close();
}

blob::blob() : p_(0), octets_(0), pBlob_(0)
{
}

blob::blob( std::size_t octets, const int8_t *p ) : p_( p ), octets_( octets ), pBlob_(0)
{
}

blob::blob( std::size_t octets, const char *p ) : p_( reinterpret_cast< const int8_t *>(p) ), octets_( octets ), pBlob_(0)
{
}

boost::uint32_t
blob::size() const
{
    return static_cast<uint32_t>(octets_);
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
    if ( pBlob_ && ( sqlite3_blob_read( pBlob_, pbuf, int(bufsize), int(offset) ) == SQLITE_OK ) )
        return true;
    return false;
}

bool
blob::write( const int8_t * pbuf, std::size_t octets, std::size_t offset ) const
{
    return sqlite3_blob_write( pBlob_, pbuf, int(octets), static_cast<int>(offset) ) == SQLITE_OK;
}

///////////////////
