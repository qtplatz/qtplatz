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
    void findPeaks();
    std::string make_outfname( const std::string& infile, const std::string& suffix );
    void set_resolution( double );

private:
    std::string outdir_;
    std::string infile_;
    std::unique_ptr< adtextfile::time_data_reader > reader_;
    adcontrols::CountingHistogram hgrm_;
    double resolution_;
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
            ( "hist,h",      "histogram outfile" )
            ( "stat,s",      "peak statistics outfile" )
            ( "directory,C", po::value< std::string >(), "result output directory" )
            ( "res",         po::value< double >(),  "peak merge resolution (ns)" )
            ( "samp-rate",   po::value< double >()->default_value( 1 ),  "digitizer sampling rate (xIncrement, ns)" )
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

                if ( vm.count( "res" ) )
                    summary.set_resolution( vm[ "res" ].as< double >() * 1.0e-9 );

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

                    summary.report( std::cout );
                }

            } else {
                std::cout << "#file: " << file << " does not reconginsed as time_data file" << std::endl;
            }
        }
    }

}


Summary::Summary( std::unique_ptr< adtextfile::time_data_reader >&& reader ) : reader_( std::move( reader ) )
                                                                             , resolution_( 0 )
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
    pf << "set terminal x11" << std::endl;
    pf << "plot \"" << file << "\" using 1:2" << std::endl;
}

void
Summary::compute_statistics( double xIncrement )
{
    hgrm_.clear();

    for ( auto& trig: reader_->data() ) {

        hgrm_ << trig;
        
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

    for ( auto& pklist: hgrm_ /* pklists_ */ ) {

        apex   = counting::statistics< CountingPeak::pk_apex >()( pklist.second.begin(), pklist.second.end() );
        height = counting::statistics< CountingPeak::pk_height >()( pklist.second.begin(), pklist.second.end() );
        area   = counting::statistics< CountingPeak::pk_area >()( pklist.second.begin(), pklist.second.end() );
        front  = counting::statistics< CountingPeak::pk_front >()( pklist.second.begin(), pklist.second.end() );
        back   = counting::statistics< CountingPeak::pk_back >()( pklist.second.begin(), pklist.second.end() );

        of << boost::format( "%.9le\t%d" )
            % pklist.first // tof
            % pklist.second.size(); // count
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

void
Summary::findPeaks()
{
    if ( hgrm_.size() > 3 ) {
        for ( auto it = hgrm_.begin() + 1; it != hgrm_.end() - 1; ++it ) {
            // double x = it[ -1 ].first;
            // double y = it[ -1 ].size();
            // auto d = ( -it[ -1 ].apex().second + it[ 1 ].apex().second ) / ( -it[ 1 ].apex().first + it[ -1 ].apex().first )
        }
    }
}
