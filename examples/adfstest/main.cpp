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

#include <adfs/adfs.h>
#include <adfs/streambuf.h>
#include <adfs/adsqlite.h>
#include <adcontrols/massspectrum.h>

#include <iostream>
#include <fstream>
#include <boost/cstdint.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/variant.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <sstream>

#if defined _DEBUG
//#     pragma comment(lib, "adportabled.lib")  // static
//#     pragma comment(lib, "adplugind.lib")    // dll
#     pragma comment(lib, "adcontrolsd.lib")  // static
//#     pragma comment(lib, "adutilsd.lib")     // static
//#     pragma comment(lib, "acewrapperd.lib")  // static
//#     pragma comment(lib, "qtwrapperd.lib")   // static
//#     pragma comment(lib, "adutilsd.lib")     // static
#else

#endif


struct column_print : public boost::static_visitor<void> {
    template<typename T> void operator()( T& t ) const {
        std::cout << t;
    }
};

template<> void column_print::operator ()( adfs::blob& ) const
{
    std::cout << "blob";
}

template<> void column_print::operator ()( std::wstring& t ) const
{
    std::wcout << t;
}

template<> void column_print::operator ()( adfs::null& ) const
{
    std::cout << "null";
}

template<> void column_print::operator ()( adfs::error& ) const
{
    std::cout << "error";
}

void
bind_data( adfs::stmt& sql, int oid, unsigned long npos, boost::int64_t uptime, unsigned long events, std::size_t szBlob, const unsigned long * blob )
{
    sql.bind( 1 ) = oid;
    sql.bind( 2 ) = npos;
    sql.bind( 3 ) = uptime;
    sql.bind( 4 ) = events;
    sql.bind( 5 ) = adfs::blob( szBlob, reinterpret_cast< const boost::uint8_t * >(blob) );

   if ( sql.step() == adfs::sqlite_done )
       sql.reset();
}

size_t
test_data_write( adfs::sqlite& db, const size_t nbrSamples )
{
    adfs::stmt sql0( db );
    adfs::stmt sql1( db );

    bool rc;
    rc = sql0.exec( "create table data0 (oid INTEGER, npos INTEGER, time INTEGER, events INTEGER, data BLOB)");
    // rc = sql0.exec( "create table data1 (oid INTEGER, npos INTEGER, time INTEGER, events INTEGER, data BLOB)");

    unsigned long npos = 2345;
    boost::int64_t uptime = 0x100000000LL;
    unsigned long events = 1;
    // const int nbrSamples = 128 * 1024;  // 128kp
    std::size_t szBlob = nbrSamples * sizeof(long);
    boost::scoped_array<unsigned long> blob( new unsigned long[ nbrSamples ] );
    for ( size_t i = 0; i < nbrSamples; ++i )
        blob.get()[i] = i + 1;

    sql0.prepare( "INSERT INTO data0 VALUES(:oid,:npos,:time,:events,:data)" );

    sql0.begin();
    size_t nSpectra = 0;    
    for ( int i = 0; i < 1024; ++i ) {
        blob.get()[0] = 10000 + i;
        blob.get()[ nbrSamples - 1 ] = 10000 + i;
        bind_data( sql0, 1, npos + i, uptime + i, events, szBlob, blob.get() );
        nSpectra++;

        blob.get()[0] = 20000 + i;
        blob.get()[ nbrSamples - 1 ] = 20000 + i;
        bind_data( sql0, 2, npos + i, i, events, szBlob, blob.get() );
        nSpectra++;
    }
    sql0.commit();
    return nSpectra;
}

size_t
test_data_read( adfs::sqlite& db, int oid )
{
    adfs::stmt sql( db );

    // sql0.prepare( "INSERT INTO data0 VALUES(:oid,:npos,:time,:events,:data)" );
    sql.prepare( "SELECT rowid, oid, npos, time, events FROM data0 WHERE oid = :oid" );
    bool ok = sql.bind( 1 ) = oid;
    assert(ok);

    const size_t nbrSamples = 128 * 1024;
    boost::scoped_array<unsigned long> pbuf( new unsigned long [ nbrSamples ] );
    adfs::blob blob; // sqlite3_blob *pBlob = 0;
    size_t nSpectra = 0;
    while ( sql.step() == adfs::sqlite_row ) {
        std::size_t size = sql.column_count();
        (void)size;
     
        boost::int64_t irow;
        adfs::column_value_type value = sql.column_value(0);
        irow = boost::get<boost::int64_t>( value );

        //boost::int32_t oid = boost::get<boost::int64_t>( sql.column_value(1) );
        //boost::int32_t npos = boost::get<boost::int64_t>( sql.column_value(2) );
        //boost::int64_t time = boost::get<boost::int64_t>( sql.column_value(3) );

        bool rc;
        if ( ! blob ) {
            rc = blob.open( db, "main", "data0", "data", irow, adfs::readonly );
        } else {
            rc = blob.reopen( irow );
        }
        std::size_t nSize = blob.size() / sizeof( long );
        assert( nSize == nbrSamples );
        rc = blob.read( reinterpret_cast<boost::int8_t *>(pbuf.get()), nbrSamples * sizeof(long), 0 );
        unsigned long d0 = pbuf.get()[0];
        unsigned long d1 = pbuf.get()[ nbrSamples - 1 ];
        assert( ( d0 == d1 ) && d0 == (oid * 10000) + nSpectra );
        ++nSpectra;
    };
    return nSpectra;
}

size_t
test_disk_write( const std::string& file, const size_t nbrSamples )
{
    std::ofstream ofile( file.c_str(), std::ios_base::binary | std::ios_base::trunc );

    //std::size_t szBlob = nbrSamples * sizeof(long);
    boost::scoped_array<unsigned long> blob( new unsigned long[ nbrSamples ] );
    for ( size_t i = 0; i < nbrSamples; ++i )
        blob.get()[i] = i + 1;

    size_t nSpectra = 0;
    for ( int i = 0; i < 1024; ++i ) {
        blob.get()[0] = 10000 + i;
        blob.get()[ nbrSamples - 1 ] = 10000 + i;
        ofile.write( reinterpret_cast< const char *>(blob.get()), nbrSamples * sizeof(long) );
        nSpectra++;

        blob.get()[0] = 20000 + i;
        blob.get()[ nbrSamples - 1 ] = 20000 + i;
        ofile.write( reinterpret_cast< const char *>(blob.get()), nbrSamples * sizeof(long) );
        nSpectra++;
    }
    return nSpectra;
}

size_t
test_disk_read( const std::string& file, const size_t nbrSamples )
{
    std::ifstream ifile( file.c_str(), std::ios_base::binary );

    //std::size_t szBlob = nbrSamples * sizeof(long);
    boost::scoped_array<unsigned long> blob( new unsigned long[ nbrSamples ] );

    size_t nSpectra = 0;
    for ( int i = 0; i < 1024; ++i ) {
        ifile.read( reinterpret_cast< char * >( blob.get() ), nbrSamples * sizeof(long) );
        if ( ! (( blob.get()[0] == blob.get()[ nbrSamples - 1 ] ) && ( blob.get()[0] == 10000 + i ) ) )
            std::cout << "data read missmatch" << std::endl;

        nSpectra++;


        ifile.read( reinterpret_cast< char * >( blob.get() ), nbrSamples * sizeof(long) );
        if ( ! (( blob.get()[0] == blob.get()[ nbrSamples - 1 ] ) && ( blob.get()[0] == 20000 + i ) ) )
            std::cout << "data read missmatch" << std::endl;

        nSpectra++;
    }
    return nSpectra;
}


struct time_duration_counter {
    time_duration_counter() : t0( boost::posix_time::microsec_clock::local_time() ) {
    }
    boost::posix_time::ptime t0;
    boost::posix_time::time_duration duration() {
        return boost::posix_time::time_duration( boost::posix_time::microsec_clock::local_time() - t0 );
    }
};

boost::int64_t
write_test( adfs::sqlite& db, std::ofstream& log, size_t nbrSamples, int pagesize, bool prealloc, size_t& nSpectra )
{
    int rc;
    if ( pagesize ) {
        adfs::stmt sql( db );
        std::ostringstream o;
        o << "PRAGMA page_size = " << pagesize;
        // const char * page_size = "PRAGMA page_size = 8192";
        rc = sql.exec( o.str().c_str() );
        log << o.str() << ", rc=" << rc << std::endl;
    } while(0);

    if ( prealloc ) {
        time_duration_counter duration;

        adfs::stmt sql( db );
        rc = sql.exec( "create table large (a)" );
        rc = sql.exec( "insert into large values( zeroblob(512 * 1024 * 1024) )" );
        rc = sql.exec( "insert into large values( zeroblob(512 * 1024 * 1024) )" );
        rc = sql.exec( "drop table large" );
        log << "pre-allcation time:= " << duration.duration() << std::endl;
    }

    time_duration_counter duration;

    nSpectra = test_data_write( db, nbrSamples );

    boost::posix_time::time_duration td = duration.duration();
    log << "sqlite3 write " << nSpectra << " data: time=\t" << td << "\t" << nSpectra / ( double( td.total_nanoseconds() ) / 1.0e9 ) << "spectra/s" << std::endl;

    return td.total_nanoseconds();
}

boost::int64_t
read_test( adfs::sqlite& db, std::ofstream& log, size_t nbrSamples, size_t& nSpectra )
{
    (void)nbrSamples;
    time_duration_counter duration;

    nSpectra = test_data_read( db, 1 );
    nSpectra += test_data_read( db, 2 );

    boost::posix_time::time_duration td = duration.duration();
    log << "sqlite3  read " << nSpectra << " data: time=\t" << td << "\t" << nSpectra / ( double( td.total_nanoseconds() ) / 1.0e9 ) << "spectra/s" << std::endl;

    return td.total_nanoseconds();
}

boost::int64_t
disk_write_test( const std::string& filename, std::ofstream& log, size_t nbrSamples, size_t& nSpectra )
{
    time_duration_counter duration;

    nSpectra = test_disk_write( filename, nbrSamples );

    boost::posix_time::time_duration td = duration.duration();
    log << "fstream write " << nSpectra << " data: time=\t" << td << "\t" << nSpectra / ( double( td.total_nanoseconds() ) / 1.0e9 ) << "spectra/s" << std::endl;

    return td.total_nanoseconds();
}

boost::int64_t
disk_read_test( const std::string& filename, std::ofstream& log, size_t nbrSamples, size_t& nSpectra )
{
    time_duration_counter duration;

    nSpectra = test_disk_read( filename, nbrSamples );

    boost::posix_time::time_duration td = duration.duration();
    log << "fstream  read " << nSpectra << " data: time=\t" << td << "\t" << nSpectra / ( double( td.total_nanoseconds() ) / 1.0e9 ) << "spectra/s" << std::endl;

    return td.total_nanoseconds();
}

double
report( std::ofstream& log, const std::vector<double>& vec, const char * heading, size_t nSpectra )
{
    using namespace boost::accumulators;

    boost::accumulators::accumulator_set<double
        , boost::accumulators::stats< boost::accumulators::tag::mean, boost::accumulators::tag::variance> > acc;

    for ( std::vector<double>::const_iterator it = vec.begin(); it != vec.end(); ++it )
        acc( *it / 1.0e9 ); // ns -> s

    double mean = extract::mean( acc );
    log << heading << "\t mean: " << mean << "s\t sd: " << std::sqrt( extract::variance( acc ) ) << " rate:\t"
        << double(nSpectra) / mean << "spectra/seconds" << std::endl;

    return mean;
}

void
sqlite_access_test()
{
    const size_t nbrSamples = 128 * 1024;

    std::ofstream of( "adfs.log", std::ios_base::app );

    std::vector< double > td_diskw, td_diskr, td_dbw, td_dbr, td_dbw2;

    size_t nSpectra = 0;

    for ( int i = 0; i < 5; ++i ) {
        td_diskw.push_back( double( disk_write_test( "data.bin", of, nbrSamples, nSpectra ) ) );
        td_diskr.push_back( double( disk_read_test( "data.bin", of, nbrSamples, nSpectra ) ) );
    }
    double tdiskw = report( of, td_diskw, "disk write:", nSpectra );
    double tdiskr = report( of, td_diskr, "disk read :", nSpectra );

    for ( int i = 0; i < 5; ++i ) {

        boost::filesystem::remove( "disk.adfs" );
        do {
            adfs::sqlite db;
            db.open( "disk.adfs" );

            td_dbw.push_back( double( write_test( db, of, nbrSamples, 1024 * 8, false, nSpectra ) ) );
            td_dbr.push_back( double( read_test( db, of, nbrSamples, nSpectra ) ) );
            db.close();

        } while(0);
    }

    double tdbw = report( of, td_dbw, "db write:", nSpectra );
    double tdbr = report( of, td_dbr, "db read :", nSpectra );

    for ( int i = 0; i < 5; ++i ) {
        boost::filesystem::remove( "disk.adfs" );
        adfs::sqlite db;
        db.open( "disk.adfs" );

        td_dbw2.push_back( double( write_test( db, of, nbrSamples, 1024 * 8, true, nSpectra ) ) );
        td_dbr.push_back( double( read_test( db, of, nbrSamples, nSpectra ) ) );

        db.close();
    }
    double tdbw2 = report( of, td_dbw2, "db write(pre-alloc):", nSpectra );
    tdbr = report( of, td_dbr, "db read :", nSpectra );

    of << "db/disk write speed reatio: " << tdiskw / tdbw << std::endl;
    of << "db/disk write w/ pre-alloc speed reatio: " << tdiskw / tdbw2 << std::endl;
    of << "db/disk read speed reatio: " << tdiskr / tdbr << std::endl;
}

void
filesystem_test()
{

    try {
        adfs::portfolio portfolio;
        portfolio.create( L"fs.adfs" );
    } catch ( adfs::exception& ex ) {
        std::cout << ex.message << " on " << ex.category << std::endl;
    }

    adfs::portfolio portfolio;
    if ( portfolio.mount( L"fs.adfs" ) ) {
        portfolio.addFolder( L"/Acuiqre" );
        portfolio.addFolder( L"/Processed" );
        portfolio.addFolder( L"/Processed/Chromatograms" );

        adfs::folder spectra = portfolio.addFolder( L"/Processed/Spectra" );

        adfs::folium spectrum1 = spectra.addFolium( adfs::create_uuid() );

        adfs::streambuf buf;
        std::ostream ostm( &buf );

        adcontrols::MassSpectrum ms;
        ms.resize( 64 * 1024 );
        ms.archive( ostm );
        spectrum1.write( buf );

        ms.resize( 128 * 1024 );
        ms.archive( ostm );
        spectrum1.write( buf );

        adfs::folium att1 = spectrum1.addAttachment( adfs::create_uuid() );
        adfs::folium att2 = spectrum1.addAttachment( adfs::create_uuid() );
    }
}

int
main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);

    // sqlite_access_test();
    filesystem_test();

/*
    sql.prepare( "select * from data0" );
    while ( sql.step() == adfs::sqlite_row ) {
        std::size_t size = sql.column_count();
        std::cout << "\ncolumn_count=" << size << std::endl;
        for ( std::size_t i = 0; i < size; ++i ) {
            boost::apply_visitor( column_print(), sql.column_value(i) );
        }
    };
*/
}
