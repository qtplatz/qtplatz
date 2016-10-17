/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "acqrsdata.hpp"
#include <adplugins/adtextfile/time_data_reader.hpp>
#include <adcontrols/countinghistogram.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <QCoreApplication>
#include <iostream>
#include <fstream>
#include <ratio>
#include <iomanip>
#if OPENCV
# include <cv.h>
# include <opencv2/cvconfig.h>
# include <opencv2/flann/flann.hpp>
using namespace cv;
# include <opencv2/flann/hdf5.h>
#endif

namespace po = boost::program_options;

class SQLImport {
    adfs::sqlite sqlite_;
public:
    SQLImport()
        {}
    void import( const std::string& file );
    void insert( const adcontrols::CountingData& );
};

class Summary {
    Summary( const Summary& ) = delete;
    Summary& operator = ( const Summary& ) = delete;
public:
    Summary();

    void compute_statistics( double xIncrement );

    void report( std::ostream& );
    void print_histogram( const std::string& file );
    void print_statistics( const std::string& file );
    void pivot( const std::string& file );
    void findPeaks();
    std::string make_outfname( const std::string& infile, const std::string& suffix );
    void set_resolution( double );
    void set_threshold( double );
    void add( const adcontrols::CountingData& );
private:
    std::string outdir_;
    std::string infile_;
    adcontrols::CountingHistogram hgrm_;
    double resolution_;
    double threshold_;
};

int
main(int argc, char *argv[])
{
    QCoreApplication a( argc, argv );
    
    po::variables_map vm;
    po::options_description description( "counting" );
    {
        description.add_options()
            ( "help,h",      "Display this help message" )
            ( "args",        po::value< std::vector< std::string > >(),  "input files" )
            ( "hist",        "histogram outfile" )
            ( "stat,s",      "peak statistics outfile" )
            ( "directory,C", po::value< std::string >(), "result output directory" )
            ( "samp-rate",   po::value< double >()->default_value( 1 ),  "digitizer sampling rate (xIncrement, ns)" )
            ( "peak",        po::value< double >(),  "specify time-of-flight in microseconds" )
            ( "resolution",  po::value< double >(),  "peak width (ns)" )
            ( "threshold",   po::value< double >()->default_value( 15.0 ),  "threshold (mV)" )
            ( "polarity",    po::value< std::string >()->default_value( "positive" ),  "polarity (positive|negative)" )
            ( "sqlite",      "import to SQLite" )
            ( "pivot",       "Pivotting data" )
            ;

        po::positional_options_description p;
        p.add( "args",  -1 );
        po::store( po::command_line_parser( argc, argv ).options( description ).positional(p).run(), vm );
        po::notify(vm);
    }

    if ( vm.count( "help" ) ) {
        std::cout << description;
        return 0;
    }

    bool f_directory( false );

    auto cwd = boost::filesystem::current_path();

    if ( vm.count( "directory" ) ) {
        boost::filesystem::path cdir( vm[ "directory" ].as< std::string >() );
        if ( !boost::filesystem::exists( cdir ) )
            boost::filesystem::create_directories( cdir );
        if ( boost::filesystem::exists( cdir ) && !boost::filesystem::is_directory( cdir ) ) {
            std::cerr << "Directory " << cdir << " is not a directory." << std::endl;
            return -1;
        }
        boost::filesystem::current_path( cdir );
        f_directory = true;
    }
    
    if ( vm.count("args") ) {
        
        for ( auto& _file: vm[ "args" ].as< std::vector< std::string > >() ) {
            
            std::string file = f_directory ? boost::filesystem::canonical( _file, cwd ).string() : _file;
            
            boost::filesystem::path path( file );
            if ( path.extension() == ".adfs" ) {

                acqrsdata d;
                d.setThreshold( vm[ "threshold" ].as< double >() / std::milli::den );
                d.setPolairty( vm[ "polarity" ].as< std::string >() == "positive" ?
                               acqrsdata::positive_polarity : acqrsdata::negative_polarity );

                if ( d.open( path ) ) {
                    d.processIt( []( size_t idx, size_t total ){
                            std::cerr << "\rprocessed: " << idx << "/" << total;
                        });
                    std::cerr << std::endl;
                }
                
            } else {
                
                std::string adfsname;
                if ( adtextfile::time_data_reader::is_time_data( file, adfsname ) ) {
                    
                    Summary summary;
                    
                    if ( ! adfsname.empty() ) {            
                        
                        double acclVoltage(0), tDelay(0), fLength(0);
                        std::string spectrometer;
                        
                        if ( adtextfile::time_data_reader::readScanLaw( adfsname, acclVoltage, tDelay, fLength, spectrometer ) ) {
                            
                            std::cout << "#datafile: " << file << " <- " << adfsname << std::endl;
                            std::cout << "#\taccelerator voltage: " << acclVoltage
                                      << "\ttDelay: " << tDelay
                                      << "\tfLength: " << fLength
                                      << std::endl;
                            std::cout << "#\tSpectrometer: " << spectrometer << std::endl;
                            
                        }
                    }
                    
                    if ( vm.count( "sqlite" ) ) {
                        
                        SQLImport importer;
                        importer.import( file );
                        
                    } else {
                        
                        if ( vm.count( "resolution" ) )
                            summary.set_resolution( vm[ "resolution" ].as< double >() * 1.0e-9 );
                        
                        if ( vm.count( "threshold" ) )
                            summary.set_threshold( vm[ "threshold" ].as< double >() / std::milli::den );
                        
                        size_t processed(0);
                        if ( adtextfile::time_data_reader::load(
                                 file
                                 , [&]( size_t numerator, size_t denominator, const adcontrols::CountingData& d ){
                                     if ( ( processed++ % 1000 ) == 0 )
                                         std::cerr << "Processing: " << file
                                                   << boost::format( "\t%.1f%%\r") % (double( numerator ) * 100 / double(denominator) );
                                     summary.add( d );
                                     return true;
                                 }) ) {
                            std::cerr << std::endl;
                            
                            if ( vm.count( "hist" ) ) {
                                auto histfile = summary.make_outfname( file, "_hist" );
                                summary.compute_statistics( vm[ "samp-rate" ].as<double>() / std::nano::den );
                                summary.print_histogram( histfile );
                            }
                            
                            if ( vm.count( "stat" ) ) {
                                auto statfile = summary.make_outfname( file, "_stat" );
                                summary.compute_statistics( vm[ "samp-rate" ].as<double>() / std::nano::den );
                                summary.print_statistics( statfile );
                            }
                            
                            if ( !vm.count( "hist" ) && !vm.count( "stat" ) ) {
                                // assume --stat if no process specified
                                auto statfile = summary.make_outfname( file, "_stat" );
                                
                                summary.compute_statistics( vm[ "samp-rate" ].as<double>() / std::nano::den );
                                summary.print_statistics( statfile );
                            }
                            
                            summary.findPeaks();
                            summary.report( std::cout );
                            
                            if ( vm.count( "pivot" ) ) {
                                auto pivotfile = summary.make_outfname( file, "_pivot" );
                                summary.pivot( pivotfile );
                            }
                        }
                    }
                } else {
                    std::cout << "#file: " << file << " does not reconginsed as time_data file" << std::endl;
                }
            }
        }
    }

}

Summary::Summary() : resolution_( 0 )
                   , threshold_( 0 )
{
}

void
Summary::report( std::ostream& out )
{
    // auto& data = reader_->data();
    // size_t total_peaks = std::accumulate( data.begin(), data.end(), size_t(0)
    //                                       , []( size_t count, const adcontrols::CountingData& d ){
    //         return d.peaks().size() + count;
    //     });
    // std::cout << "total triggers: " << data.size() << "\ttotal peaks\t" << total_peaks << std::endl;
}

void
Summary::print_histogram( const std::string& file )
{
    std::ofstream of( file );

    for ( auto& pk: hgrm_ )
        of << boost::format( "%.9le\t%d" ) % pk.first % pk.second.size() << std::endl;

    boost::filesystem::path plt( file );
    plt.replace_extension( ".plt" );
    std::ofstream pf( plt.string() );
    // pf << "set terminal x11" << std::endl;
    pf << "plot \"" << file << "\" using 1:2" << std::endl;
}

void
Summary::add( const adcontrols::CountingData& d )
{
    if ( ! d.peaks().empty() )
        hgrm_ << d;
}

void
Summary::compute_statistics( double xIncrement )
{
    std::cerr << "computing statistics" << std::endl;
    
    for ( auto& pklist: hgrm_ ) {
        auto& peaks = pklist.second;
        size_t sz = peaks.size();
        auto it = std::remove_if( peaks.begin(), peaks.end(), [&]( const auto& pk ){ return pk.apex().second > threshold_; } ); // NEG peak
        peaks.erase( it, peaks.end() );
        std::sort( peaks.begin(), peaks.end(), []( const auto& a, const auto& b ){ return a.apex().second > b.apex().second; } );
    }
}

void
Summary::print_statistics( const std::string& file )
{
    std::string xfile( file );
    int id(1);
    while( boost::filesystem::exists( xfile ) )
        xfile = ( boost::format( "%s-%d" ) % file % id++ ).str();
        
    std::ofstream of( xfile );
    using namespace adcontrols;

    counting::Stat apex, height, area, front, back;

    double t = 0;
    for ( auto& pklist: hgrm_ /* pklists_ */ ) {

        apex   = counting::statistics< CountingPeak::pk_apex >()( pklist.second.begin(), pklist.second.end() );
        height = counting::statistics< CountingPeak::pk_height >()( pklist.second.begin(), pklist.second.end() );
        area   = counting::statistics< CountingPeak::pk_area >()( pklist.second.begin(), pklist.second.end() );
        front  = counting::statistics< CountingPeak::pk_front >()( pklist.second.begin(), pklist.second.end() );
        back   = counting::statistics< CountingPeak::pk_back >()( pklist.second.begin(), pklist.second.end() );

        of << boost::format( "%.9le\t%d" )
            % pklist.first // tof
            % pklist.second.size(); // count
        // of << boost::format( "\tdt:\t%9.4lf" ) % (( pklist.first - t ) * 1.0e9);
        t = pklist.first;
        of << boost::format( "\tV:\t%9.4lf\t%9.3lf" ) % apex.mean % apex.stddev;
        of << boost::format( "\tH:\t%9.4lf\t%9.3lf" ) % height.mean % height.stddev;
        of << boost::format( "\tA:\t%9.4lf\t%9.3lf" ) % area.mean % area.stddev;
        of << boost::format( "\tF:\t%9.4lf\t%9.3lf" ) % front.mean % front.stddev;
        of << boost::format( "\tB:\t%9.4lf\t%9.3lf" ) % back.mean % back.stddev;
        of << std::endl;
    }

    std::cout << "statistics reported on file: " << file << std::endl;

    // boost::filesystem::path plt( file );
    // plt.replace_extension( ".plt" );
    // std::ofstream pf( plt.string() );
    // pf << "set terminal x11" << std::endl;
    // pf << "plot \"" << file << "\" using 1:2" << std::endl;
}

void
Summary::set_resolution( double res )
{
    resolution_ = res;
}

void
Summary::set_threshold( double th )
{
    threshold_ = th;
    std::cout << "set_threshold( " << th << ")" << std::endl;
}

std::string
Summary::make_outfname( const std::string& infile, const std::string& suffix )
{
    int id(1);
    auto stem = boost::filesystem::path( infile ).stem();
    stem += suffix;

    if ( std::abs( threshold_ ) >= 1.0e-6 )
        stem += ( boost::format( "_%.1fmV" ) % threshold_ ).str();

    if ( outdir_.empty() ) {
        std::string path = stem.string() + ".csv";
        while ( boost::filesystem::exists( path ) )
            path = ( boost::format( "%s~%d.csv" ) % stem.string() % id++ ).str();
        return path;
    } else {
        auto name = boost::filesystem::path( outdir_ ) / stem;
        std::string path = name.string() + ".csv";
        while ( boost::filesystem::exists( path ) ) {
            path = ( boost::format( "%s~%d.csv" ) % name.string() % id++ ).str();
        }
        return path;
    }
        
}

void
Summary::findPeaks()
{
}

void
Summary::pivot( const std::string& file )
{
    std::string xfile( file );
    int id(1);
    while( boost::filesystem::exists( xfile ) )
        xfile = ( boost::format( "%s-%d" ) % file % id++ ).str();
        
    std::ofstream of( xfile );
    using namespace adcontrols;

    for ( const auto& pklist: hgrm_ ) {
        for( auto& peak: pklist.second ) {
            of << std::fixed << std::setprecision(8)
               << pklist.first
               << "\t" << peak.apex().first << "\t" << peak.apex().second
               << "\t" << peak.front().first << "\t" << peak.front().second
               << "\t" << peak.back().first << "\t" << peak.back().second
               << std::endl;
        }
    }
}

void
SQLImport::import( const std::string& file )
{
    auto stem = boost::filesystem::path( file ).stem();
    
    boost::filesystem::path dbf( stem );
    dbf.replace_extension( ".sqlite3" );
    
    // if ( boost::filesystem::exists( dbf ) )
    //     boost::filesystem::remove( dbf );
    
    if ( sqlite_.open( dbf.string().c_str(), adfs::opencreate ) ) {
        adfs::stmt sql( sqlite_ );

        sql.exec( "PRAGMA synchronous = OFF" );
        sql.exec( "PRAGMA journal_mode = MEMORY" );
        sql.exec( "PRAGMA page_size = 8192" );

        sql.exec(
            "CREATE TABLE \
trigger (                 \
id INTEGER PRIMARY KEY    \
, protocol INTEGER        \
, timeSinceEpoch INTEGER  \
, elapsedTime REAL        \
, events INTEGER          \
, threshold REAL          \
, algo INTEGER )" );

        sql.exec(
            "CREATE TABLE \
peak (                    \
idTrigger INTEGER         \
,peak_time REAL           \
,peak_intensity REAL      \
,front_offset INTEGER     \
,front_intensity REAL     \
,back_offset INTEGER      \
,back_intensity REAL      \
,FOREIGN KEY( idTrigger ) REFERENCES trigger( id ))" );

        size_t processed( 0 );
        adtextfile::time_data_reader::load(
            file
            , [&]( size_t numerator, size_t denominator, const adcontrols::CountingData& d ){
                if ( ( processed++ % 1000 ) == 0 )
                    std::cerr << "Processing: " << file
                              << boost::format( "\t%.1f%%\r") % (double( numerator ) * 100 / double(denominator) );
                insert( d );
                return true;
            });
    }
}

void
SQLImport::insert( const adcontrols::CountingData& d )
{
    do {
        adfs::stmt sql( sqlite_ );
        
        sql.prepare( "INSERT INTO trigger ( id,protocol,timeSinceEpoch,elapsedTime,events,threshold,algo ) VALUES (?,?,?,?,?,?,?)" );
        int id(1);
        sql.bind( id++ ) = d.triggerNumber();
        sql.bind( id++ ) = d.protocolIndex();
        sql.bind( id++ ) = d.timeSinceEpoch();
        sql.bind( id++ ) = d.elapsedTime();
        sql.bind( id++ ) = d.events();
        sql.bind( id++ ) = d.threshold();
        sql.bind( id++ ) = d.algo();
        if ( sql.step() != adfs::sqlite_done ) {
            ADDEBUG() << "sql error";
            return;
        }
    } while ( 0 );

    do {
        adfs::stmt sql( sqlite_ );
        
        sql.prepare( "INSERT INTO peak ( idTrigger,peak_time,peak_intensity,front_offset,front_intensity,back_offset,back_intensity )\
 VALUES (?,?,?,?,?,?,?)" );
        for ( auto& pk: d.peaks() ) {
            int id = 1;
            sql.bind( id++ ) = d.triggerNumber();
            sql.bind( id++ ) = pk.apex().first;
            sql.bind( id++ ) = pk.apex().second;
            sql.bind( id++ ) = pk.front().first;
            sql.bind( id++ ) = pk.front().second;
            sql.bind( id++ ) = pk.back().first;
            sql.bind( id++ ) = pk.back().second;
            if ( sql.step() != adfs::sqlite_done ) {
                ADDEBUG() << "sql error";
                return;
            }        
        }
    } while ( 0 );
}
