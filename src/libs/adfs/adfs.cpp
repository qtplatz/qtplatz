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
#include "sqlite3.h"
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace adfs;

namespace adfs { namespace detail {
  
    class storage {
        sqlite3 * db_;
    public:
        ~storage();
        storage();
        inline operator sqlite3 * () { return db_; }

        bool create( const char * filename );
        bool close();
        // 
        static int callback( void * NotUsed, int argc, char ** argv, char ** azColName );

        struct ErrMsg {
            char * p;
            inline operator char ** () { return &p; }
            ErrMsg() : p(0) {}
            ~ErrMsg() { sqlite3_free( p ); }
        };
    };

    
}
}

//////

storage::storage() : impl_(0)
{
}

storage::storage( const storage& t ) : impl_( t.impl_ )
{
}

storage::~storage()
{
    delete impl_;
}

bool
storage::create( const char * filename, bool readonly )
{
    (void)readonly;

    impl_ = new detail::storage;
    return impl_->create( filename );
}

bool
storage::close()
{
    return impl_ && impl_->close();
}

////////////////////
////////////////////

detail::storage::storage() : db_(0)
{
}

detail::storage::~storage()
{
  if ( db_ )
      sqlite3_close( db_ );
}

bool
detail::storage::close()
{
    if ( db_ )
        sqlite3_close( db_ );
    db_ = 0;
    return true;
}

bool
detail::storage::create( const char * filename )
{
    boost::filesystem::remove( filename );

    int rc = sqlite3_open( filename, &db_ );
    if ( rc )
        return false;

    ErrMsg msg;
    rc = sqlite3_exec( db_, "create table t1 (t1key INTEGER PRIMARY KEY, data TEXT, num double, timeEnter DATE)", callback, 0, msg );
    if ( rc != SQLITE_OK ) {
        long x = 0; // error
    }

    rc = sqlite3_exec( db_, "insert into t1 (data, num) values ('This is sample dta', 3 )", callback, 0, msg );
    if ( rc != SQLITE_OK ) {
        long x = 0; // error
    }

    rc = sqlite3_exec( db_, "create table zTable (zkey INTEGER PRIMARY KEY, data BLOB)", callback, 0, msg );
    if ( rc != SQLITE_OK ) {
        long x = 0; // error
    }

//#if defined TRANSACTION
    const char * transaction = "BEGIN DEFERRED";
    //const char * transaction = "BEGIN IMMEDIATE";
    //const char * transaction = "BEGIN EXCLUSIVE";
    if ( sqlite3_exec( db_, transaction, callback, 0, msg ) != SQLITE_OK )
        std::cout << msg.p << std::endl;
//#endif

    sqlite3_stmt * stmt = 0;
    const char * tail = 0;
    char * sql = "INSERT INTO zTable VALUES(?,?)";

    const size_t N = 1;
    const size_t nbrSamples = 32 * 1024;
    const size_t nSpectra = 1024;

    boost::scoped_array<unsigned long> blob( new unsigned long[ N * nbrSamples ] );
    
    if ( sqlite3_prepare_v2( db_, sql, -1, &stmt, &tail ) != SQLITE_OK )
        std::cout << sqlite3_errmsg( db_ );

    boost::posix_time::ptime t0( boost::posix_time::microsec_clock::local_time() );

    for ( int k = 0; k < ( nSpectra / N ); ++k ) {
        if ( sqlite3_bind_int( stmt, 1, k ) != SQLITE_OK )
            std::cout << sqlite3_errmsg( db_ );

        if ( sqlite3_bind_blob( stmt, 2, blob.get(), N * (nbrSamples * sizeof(long)), SQLITE_STATIC) != SQLITE_OK )
            std::cout << sqlite3_errmsg( db_ );

        if ( sqlite3_step( stmt ) != SQLITE_DONE )
            std::cout << sqlite3_errmsg( db_ );
        sqlite3_reset( stmt );
    }
    sqlite3_finalize( stmt );

    if ( sqlite3_exec( db_, "COMMIT", callback, 0, msg ) != SQLITE_OK )
        std::cout << msg.p << std::endl;

    boost::posix_time::ptime t1(  boost::posix_time::microsec_clock::local_time() );
    boost::posix_time::time_duration td = t1 - t0;
    // td.total_nanoseconds();

    std::cout << "time= " << td << " " << 1024 / ( double( td.total_nanoseconds() ) / 1.0e9 ) << "spectra/s" << std::endl;
    std::ofstream of( "adfs.log", std::ios_base::app );
    of << transaction << " nbrSamples=" << nbrSamples / 1024 << " time= " << td << " " << 1024 / ( double( td.total_nanoseconds() ) / 1.0e9 ) << "spectra/s" << std::endl;
    
/*
    sqlite3_blob * pBlob = 0;
    sqlite3_int64 irow = 3;
    enum { readonly, readwrite };
    if ( sqlite3_blob_open( db_, "main", "zTable", "data", irow, readwrite, &pBlob ) != SQLITE_OK ) {
        std::cout << sqlite3_errmsg( db_ ) << std::endl;
    } else {
        unsigned long t0 = ::GetTickCount();
        for ( int k = 0; k < 1024; ++k ) {
            if ( sqlite3_blob_write( pBlob, blob, sizeof( blob ), 0 ) != SQLITE_OK )
                std::cout << sqlite3_errmsg( db_ ) << std::endl;
            sqlite3_blob_reopen( pBlob, k + 1 );
        }
        unsigned long t1 = ::GetTickCount();
        std::cout << "time= " << t1 - t0 << std::endl;
    }
*/
    return true;
}

int
detail::storage::callback( void *, int argc, char ** argv, char ** azColName )
{
    for ( int i = 0; i < argc; ++i )
        std::cout << azColName[i] << " = " << ( argv[i] ? argv[i] : "NULL" ) << std::endl;
    return 0;
}