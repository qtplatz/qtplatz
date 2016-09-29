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

#include <adplugins/adtextfile/time_data_reader.hpp>
#include <adcontrols/countinghistogram.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include <ratio>
#if OPENCV
# include <cv.h>
# include <opencv2/cvconfig.h>
# include <opencv2/flann/flann.hpp>
using namespace cv;
# include <opencv2/flann/hdf5.h>
#endif

namespace po = boost::program_options;

class Summary {
    Summary() = delete;
    Summary( const Summary& ) = delete;
    Summary& operator = ( const Summary& ) = delete;
public:
    Summary( std::unique_ptr< adtextfile::time_data_reader > && reader );

    adtextfile::time_data_reader * reader() { return reader_.get(); }

    // void compute_histogram( double xIncrement );
    void compute_statistics( double xIncrement );

    void report( std::ostream& );
    void print_histogram( const std::string& file );
    void print_statistics( const std::string& file );
    void pivot( const std::string& file );
    void findPeaks();
    std::string make_outfname( const std::string& infile, const std::string& suffix );
    void set_resolution( double );
    void set_threshold( double );

private:
    std::string outdir_;
    std::string infile_;
    std::unique_ptr< adtextfile::time_data_reader > reader_;
    adcontrols::CountingHistogram hgrm_;
    double resolution_;
    double threshold_;
};

int
main(int argc, char *argv[])
{
    po::variables_map vm;
    po::options_description description( "acqiris" );
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
            ( "threshold",   po::value< double >(),  "threshold (mV)" )
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

            std::string adfsname;
            if ( adtextfile::time_data_reader::is_time_data( file, adfsname ) ) {

                Summary summary( std::make_unique< adtextfile::time_data_reader >() );

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

                if ( vm.count( "resolution" ) )
                    summary.set_resolution( vm[ "resolution" ].as< double >() * 1.0e-9 );

                if ( vm.count( "threshold" ) )
                    summary.set_threshold( vm[ "threshold" ].as< double >() );

                if ( summary.reader()->load(
                         file
                         , [&]( size_t numerator, size_t denominator ){
                             std::cerr << "Processing: " << file
                                       << boost::format( "\t%.1f%%\r") % (double( numerator ) * 100 / double(denominator) );
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

            } else {
                std::cout << "#file: " << file << " does not reconginsed as time_data file" << std::endl;
            }
        }
    }

}


Summary::Summary( std::unique_ptr< adtextfile::time_data_reader >&& reader ) : reader_( std::move( reader ) )
                                                                             , resolution_( 0 )
                                                                             , threshold_( 0 )
{
}

void
Summary::report( std::ostream& out )
{
    auto& data = reader_->data();
    size_t total_peaks = std::accumulate( data.begin(), data.end(), size_t(0)
                                          , []( size_t count, const adcontrols::CountingData& d ){
            return d.peaks().size() + count;
        });
    std::cout << "total triggers: " << data.size() << "\ttotal peaks\t" << total_peaks << std::endl;
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
Summary::compute_statistics( double xIncrement )
{
    hgrm_.clear();
    std::cerr << "computing statistics" << std::endl;
    
    for ( auto& trig: reader_->data() )
        hgrm_ << trig;

    for ( auto& pklist: hgrm_ ) {
        auto& peaks = pklist.second;
        size_t sz = peaks.size();
        auto it = std::remove_if( peaks.begin(), peaks.end(), [&]( const auto& pk ){ return pk.apex().second > threshold_; } );
        peaks.erase( it, peaks.end() );
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
Summary::pivot( const std::string& file )
{
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

    if ( outdir_.empty() ) {
        std::string path = stem.string() + ".log";
        while ( boost::filesystem::exists( path ) )
            path = ( boost::format( "%s~%d.log" ) % stem.string() % id++ ).str();
        return path;
    } else {
        auto name = boost::filesystem::path( outdir_ ) / stem;
        std::string path = name.string() + ".log";
        while ( boost::filesystem::exists( path ) ) {
            path = ( boost::format( "%s~%d.log" ) % name.string() % id++ ).str();
        }
        return path;
    }
        
}

#if OPENCV
void
Summary::findPeaks()
{
    auto numPeaks = std::accumulate( hgrm_.begin(), hgrm_.end(), size_t(0), [](size_t x, const auto& pk ) { return x + pk.second.size(); } );

    std::vector< float > times(numPeaks);

    auto it = times.begin();
    for ( const auto& column : hgrm_ ) {
        std::transform( column.second.begin(), column.second.end(), it, []( auto& pk ){ return float( pk.apex().first ); } );
        std::advance( it, column.second.size() );
    }
    
    std::cout << "numPeaks: " << numPeaks << std::endl;

    cv::Mat features( times.size(), 1, CV_32FC1, times.data() );

#if 0
    std::vector< int > labels;
    std::vector< float > centres;

    cv::kmeans( m
                , 1
                , labels
                , cv::TermCriteria( CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 10, 5.0e-9 )
                , 3
                , cv::KMEANS_PP_CENTERS
                , centres );
    
    std::cout << "labels: " << labels.size() << ", centres: " << centres.size() << std::endl;
    
    for ( auto& c: centres )
      std::cout << c << std::endl;

    for ( auto& l: labels )
      std::cout << l << std::endl;
#endif
    
   cvflann::KMeansIndexParams params( 3, 5, cvflann::FLANN_CENTERS_RANDOM, 0.1 );
    
    typedef cv::flann::L2_Simple< float > distance_type;
    //auto index = std::make_unique< cv::flann::GenericIndex< distance_type > >( features, params );
    
    cv::Mat centres( 6, 1, CV_32FC1, float(0) );

    int count = cv::flann::hierarchicalClustering< distance_type >( features, centres, params );
    
    std::cout << "count=" << count << std::endl;
    
    std::cout << centres << std::endl;
}

#else

void
Summary::findPeaks()
{
}

#endif

