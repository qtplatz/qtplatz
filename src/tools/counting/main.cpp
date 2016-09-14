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
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include <ratio>

namespace po = boost::program_options;

struct peakstat {
    double mean, sd, min, max;
    size_t count;
    peakstat( double _1 = 0, double _2 = 0, double _3 = 0, double _4 = 0, size_t _5 = 0 )
        : mean( _1 ), sd( _2 ), min( _3 ), max( _4 ), count( _5 )
        {}
};


class Summary {
    Summary() = delete;
    Summary( const Summary& ) = delete;
    Summary& operator = ( const Summary& ) = delete;
public:
    Summary( std::unique_ptr< adtextfile::time_data_reader > && reader );

    adtextfile::time_data_reader * reader() { return reader_.get(); }

    void compute_histogram( double xIncrement );
    void compute_statistics( double xIncrement );

    void report( std::ostream& );
    void print_histogram( const std::string& file );
    void print_statistics( const std::string& file );

    bool set_outdir( const std::string& dir );
    std::string make_outfname( const std::string& infile, const std::string& suffix );

private:
    std::unique_ptr< adtextfile::time_data_reader > reader_;
    std::vector< std::pair< double, size_t > > histogram_;
    std::vector< std::pair< double, std::vector< adcontrols::CountingPeak > > > pklists_;
    std::string outdir_;
    std::string infile_;
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
            ( "odir,C",      po::value< std::string >(), "result output directory" )
            ( "samp-rate",   po::value< double >()->default_value( 1 ),  "digitizer sampling rate (xIncrement, ns)" )
            // ( "resolution",  po::value< double >()->default_value( 5 ),  "peak merge resolution (ns)" )
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
    
    if ( vm.count("args") ) {
        
        for ( auto& file: vm[ "args" ].as< std::vector< std::string > >() ) {
            
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

                if ( vm.count( "odir" ) ) {
                    if ( !summary.set_outdir( vm[ "odir" ].as< std::string >() ) ) {
                        std::cerr << "Directory " << vm[ "odir" ].as< std::string >() << " can't be created." << std::endl;
                        return 0;
                    }
                }

                if ( summary.reader()->load( file
                                             , [&]( size_t numerator, size_t denominator ){
                                                 std::cerr << "Processing: " << file
                                                           << boost::format( "\t%.1f%%\r") % (double( numerator ) * 100 / double(denominator) );
                                                 return true;
                                             }) ) {
                    std::cerr << std::endl;
                    
                    if ( vm.count( "hist" ) ) {
                        auto histfile = summary.make_outfname( file, "_hist" );
                        summary.compute_histogram( vm[ "samp-rate" ].as<double>() / std::nano::den );
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
{
}

void
Summary::report( std::ostream& out )
{
    auto& data = reader_->data();
    size_t total_peaks = std::accumulate( data.begin(), data.end(), size_t(0), []( size_t count, const adcontrols::CountingData& d ){
            return d.peaks().size() + count;
        });
    std::cout << "total triggers: " << data.size() << "\ttotal peaks\t" << total_peaks << std::endl;
}

void
Summary::compute_histogram( double xIncrement )
{
    histogram_.clear();
    
    for ( auto& trig: reader_->data() ) {

        if ( trig.peaks().size() ) {
            for ( auto& pk: trig.peaks() ) {

                if ( pk.apex().first < 1.0e-9 )
                    std::cout << "too small tof detected" << std::endl;
                
                auto it = std::lower_bound( histogram_.begin(), histogram_.end(), pk
                                            , []( const std::pair< double, size_t >& a, const adcontrols::CountingPeak& b ){
                                                return a.first < b.apex().first;
                                            });
                if ( it == histogram_.end() ) {
                    histogram_.emplace_back( pk.apex().first, 1 );
                } else if ( std::abs( it->first - pk.apex().first ) < xIncrement ) {
                    it->second++;
                } else {
                    histogram_.emplace( it, pk.apex().first, 1 );
                }
            }
        }
    }
}

void
Summary::print_histogram( const std::string& file )
{
    std::ofstream of( file );

    for ( auto& pk: histogram_ )
        of << boost::format( "%.9le, %d" ) % pk.first % pk.second << std::endl;

    boost::filesystem::path plt( file );
    plt.replace_extension( ".plt" );
    std::ofstream pf( plt.string() );
    pf << "set terminal x11" << std::endl;
    pf << "plot \"" << file << "\" using 1:2" << std::endl;
}

void
Summary::compute_statistics( double xIncrement )
{
    pklists_.clear();

    for ( auto& trig: reader_->data() ) {
        
        if ( trig.peaks().size() ) {
            for ( auto& pk: trig.peaks() ) {
                
                auto it = std::lower_bound( pklists_.begin(), pklists_.end(), pk
                                            , []( const std::pair< double, std::vector< adcontrols::CountingPeak > >& a
                                                  , const adcontrols::CountingPeak& b ){
                                                return a.first < b.apex().first;
                                            });
                if ( it == pklists_.end() ) {
                    
                    pklists_.emplace_back( pk.apex().first, std::vector< adcontrols::CountingPeak >( { pk } ) );

                } else if ( std::abs( it->first - pk.apex().first ) < xIncrement ) {
                    
                    it->second.emplace_back( pk );

                } else {

                    pklists_.emplace( it, std::make_pair( pk.apex().first, std::vector< adcontrols::CountingPeak >( { pk } ) ) );

                }
            }
        }
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

    peakstat apex, height, area, front, back;

    for ( auto& pklist: pklists_ ) {
        do {
            // absolute height
            auto range = std::minmax_element(
                pklist.second.begin(), pklist.second.end(), []( const CountingPeak& a, const CountingPeak& b ){
                    return a.apex().second < b.apex().second;
                });
            double mean = std::accumulate(
                pklist.second.begin(), pklist.second.end(), 0.0, []( double b, const CountingPeak& a ){
                    return a.apex().second + b;
                }) / pklist.second.size();
            double sd2 = std::accumulate(
                pklist.second.begin(), pklist.second.end(), 0.0, [&]( double b, const CountingPeak& a ){
                    return b + ( ( a.apex().second - mean ) * ( a.apex().second - mean ) );
                }) / ( pklist.second.size() - 1 );
            apex = peakstat( mean, std::sqrt( sd2 ), range.first->apex().second, range.second->apex().second, pklist.second.size() );
        } while(0);

        do {
            // height
            auto range = std::minmax_element(
                pklist.second.begin(), pklist.second.end()
                , []( const CountingPeak& a, const CountingPeak& b ){ return a.height() < b.height(); });
            double mean = std::accumulate(
                pklist.second.begin(), pklist.second.end(), 0.0
                , []( double b, const CountingPeak& a ){ return b + a.height(); } ) / pklist.second.size();
            double sd2 = std::accumulate(
                pklist.second.begin(), pklist.second.end(), 0.0, [&]( double b, const CountingPeak& a ){
                    return b + ( a.height() - mean ) * ( a.height() - mean );
                }) / ( pklist.second.size() - 1 );
            height = peakstat( mean, std::sqrt( sd2 ), range.first->height(), range.second->height(), pklist.second.size() );
        } while( 0 );

        do {
            // area
            auto range = std::minmax_element(
                pklist.second.begin(), pklist.second.end()
                , []( const CountingPeak& a, const CountingPeak& b ){ return a.area() < b.area(); });
            double mean = std::accumulate(
                pklist.second.begin(), pklist.second.end(), 0.0
                , []( double b, const CountingPeak& a ){ return b + a.area(); } ) / pklist.second.size();
            double sd2 = std::accumulate(
                pklist.second.begin(), pklist.second.end(), 0.0, [&]( double b, const CountingPeak& a ){
                    return b + ( a.area() - mean ) * ( a.area() - mean );
                }) / ( pklist.second.size() - 1 );
            area = peakstat( mean, std::sqrt( sd2 ), range.first->area(), range.second->area(), pklist.second.size() );
            
        } while( 0 );
        
        //////
        do {
            // front
            auto range = std::minmax_element(
                pklist.second.begin(), pklist.second.end()
                , []( const CountingPeak& a, const CountingPeak& b ){ return a.front().second < b.front().second;  });
            double mean = std::accumulate(
                pklist.second.begin(), pklist.second.end(), 0.0
                , []( double b, const CountingPeak& a ){ return a.front().second + b; }) / pklist.second.size();
            double sd2 = std::accumulate(
                pklist.second.begin(), pklist.second.end(), 0.0
                , [&]( double b, const CountingPeak& a ){ return b + ( a.front().second - mean ) * ( a.front().second - mean );
                }) / ( pklist.second.size() - 1 );
            front = peakstat( mean, std::sqrt( sd2 ), range.first->front().second, range.second->front().second, pklist.second.size() );
        } while ( 0 );

        do {
            // back
            auto range = std::minmax_element(
                pklist.second.begin(), pklist.second.end()
                , []( const CountingPeak& a, const CountingPeak& b ){ return a.back().second < b.back().second;  });            
            double mean = std::accumulate(
                pklist.second.begin(), pklist.second.end(), 0.0, []( double b, const CountingPeak& a ){
                    return a.back().second + b;
                }) / pklist.second.size();
            double sd2 = std::accumulate(
                pklist.second.begin(), pklist.second.end(), 0.0, [&]( double b, const CountingPeak& a ){
                    return b + ( a.back().second - mean ) * ( a.back().second - mean );
                }) / ( pklist.second.size() - 1 );
            back = peakstat( mean, std::sqrt( sd2 ), range.first->back().second, range.second->back().second, pklist.second.size() );
        } while (0);
        ////////
        
        of << boost::format( "%.9le,\t%d" )
            % pklist.first // tof
            % pklist.second.size(); // count
        of << boost::format( "\tV: %9.5lf, %9.5lf" ) % apex.mean % apex.sd;
        of << boost::format( "\tH: %9.5lf, %9.5lf" ) % height.mean % height.sd;
        of << boost::format( "\tA: %9.5lf, %9.5lf" ) % area.mean % area.sd;
        of << boost::format( "\tF: %9.5lf, %9.5lf" ) % front.mean % front.sd;
        of << boost::format( "\tB: %9.5lf, %9.5lf" ) % back.mean % back.sd
           << std::endl;
    }

    // boost::filesystem::path plt( file );
    // plt.replace_extension( ".plt" );
    // std::ofstream pf( plt.string() );
    // pf << "set terminal x11" << std::endl;
    // pf << "plot \"" << file << "\" using 1:2" << std::endl;
}

bool
Summary::set_outdir( const std::string& path )
{
    if ( !boost::filesystem::exists( path ) )
        boost::filesystem::create_directories( path );

    if ( boost::filesystem::exists( path ) && boost::filesystem::is_directory( path ) ) {
        outdir_ = path;
        return true;
    }
    return false;
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
        while ( boost::filesystem::exists( path ) )
            path = ( boost::format( "%s~%d.log" ) % name % id++ ).str();
        return path;
    }
        
}
