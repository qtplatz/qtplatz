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

private:
    std::unique_ptr< adtextfile::time_data_reader > reader_;
    std::vector< std::pair< double, size_t > > histogram_;
    std::vector< std::pair< double, std::vector< adcontrols::CountingPeak > > > pklists_;
};

int
main(int argc, char *argv[])
{
    po::variables_map vm;
    po::options_description description( "acqiris" );
    {
        description.add_options()
            ( "help,h",     "Display this help message" )
            ( "args",       po::value< std::vector< std::string > >(),  "input files" )
            ( "histogram",  po::value< std::string >(),                 "histogram outfile" )
            ( "stat,s",     po::value< std::string >(),                 "peak statistics outfile" )
            ( "samp-rate",  po::value< double >()->default_value( 1 ),  "digitizer sampling rate (xIncrement, ns)" )
            ( "resolution", po::value< double >()->default_value( 5 ),  "peak merge resolution (ns)" )
            ( "tof",        po::value< double >(), "count peak for given tof(us) and resolution" )
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

                    Summary summary( std::make_unique< adtextfile::time_data_reader >() );
                    
                    if ( summary.reader()->load( file ) ) {

                        if ( vm.count( "histogram" ) ) {
                            boost::filesystem::path outfile( file );
                            outfile.replace_extension( "_hist.txt" );
                            
                            summary.compute_histogram( vm[ "samp-rate" ].as<double>() / std::nano::den );
                            summary.print_histogram( vm[ "histogram" ].as< std::string >() );
                        }

                        if ( vm.count( "stat" ) ) {
                            boost::filesystem::path outfile( file );
                            outfile.replace_extension( "_stat.txt" );
                            
                            summary.compute_statistics( vm[ "samp-rate" ].as<double>() / std::nano::den );
                            summary.print_statistics( vm[ "stat" ].as< std::string >() );
                        }
                        
                        summary.report( std::cout );
                    }
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

                if ( pk.apex.first < 1.0e-9 )
                    std::cout << "too small tof detected" << std::endl;
                
                auto it = std::lower_bound( histogram_.begin(), histogram_.end(), pk
                                            , []( const std::pair< double, size_t >& a, const adcontrols::CountingPeak& b ){
                                                return a.first < b.apex.first;
                                            });
                if ( it == histogram_.end() ) {
                    histogram_.emplace_back( pk.apex.first, 1 );
                } else if ( std::abs( it->first - pk.apex.first ) < xIncrement ) {
                    it->second++;
                } else {
                    histogram_.emplace( it, pk.apex.first, 1 );
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
                                                return a.first < b.apex.first;
                                            });
                if ( it == pklists_.end() ) {
                    
                    pklists_.emplace_back( pk.apex.first, std::vector< adcontrols::CountingPeak >( { pk } ) );

                } else if ( std::abs( it->first - pk.apex.first ) < xIncrement ) {
                    
                    it->second.emplace_back( pk );

                } else {

                    pklists_.emplace( it, std::make_pair( pk.apex.first, std::vector< adcontrols::CountingPeak >( { pk } ) ) );

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

    peakstat apex, height, front, back;

    for ( auto& pklist: pklists_ ) {
        do {
            auto range = std::minmax_element(
                pklist.second.begin(), pklist.second.end(), []( const CountingPeak& a, const CountingPeak& b ){
                    return a.apex.second < b.apex.second;
                });
            double mean = std::accumulate(
                pklist.second.begin(), pklist.second.end(), 0.0, []( double b, const CountingPeak& a ){
                    return a.apex.second + b;
                }) / pklist.second.size();
            double sd = std::accumulate(
                pklist.second.begin(), pklist.second.end(), 0.0, [&]( double b, const CountingPeak& a ){
                    return ( a.apex.second - mean ) * ( a.apex.second - mean );
                }) / ( pklist.second.size() - 1 );
            apex.count = pklist.second.size();
            apex.mean = mean;
            apex.sd = sd;
            apex.min = range.first->apex.second;
            apex.min = range.second->apex.second;
        } while(0);

        do {
            // height
            auto range = std::minmax_element(
                pklist.second.begin(), pklist.second.end()
                , []( const CountingPeak& a, const CountingPeak& b ){
                    double h = ( a.back.second + a.front.second ) / 2.0;
                    return ( a.apex.second - h ) < ( b.apex.second - h );
                });
            double mean = std::accumulate(
                pklist.second.begin(), pklist.second.end(), 0.0, []( double b, const CountingPeak& a ){
                    double h = ( a.back.second + a.front.second ) / 2.0;
                    return ( a.apex.second - h ) + b;
                }) / pklist.second.size();
            double sd = std::accumulate(
                pklist.second.begin(), pklist.second.end(), 0.0, [&]( double b, const CountingPeak& a ){
                    double h = a.apex.second - ( a.back.second + a.front.second ) / 2.0;
                    return ( h - mean ) * ( h - mean );
                }) / ( pklist.second.size() - 1 );
            height.count = pklist.second.size();
            height.mean = mean;
            height.sd = sd;
            height.min = range.first->apex.second - range.first->back.second;
            height.min = range.second->apex.second - range.second->back.second;
        } while( 0 );
        //////
        do {
            // front
            double mean = std::accumulate(
                pklist.second.begin(), pklist.second.end(), 0.0, []( double b, const CountingPeak& a ){
                    return a.front.second + b;
                }) / pklist.second.size();
            double sd = std::accumulate(
                pklist.second.begin(), pklist.second.end(), 0.0, [&]( double b, const CountingPeak& a ){
                    return ( a.front.second - mean ) * ( a.front.second - mean );
                }) / ( pklist.second.size() - 1 );
            front.count = pklist.second.size();
            front.mean = mean;
            front.sd = sd;            
        } while ( 0 );
        do {
            // back
            double mean = std::accumulate(
                pklist.second.begin(), pklist.second.end(), 0.0, []( double b, const CountingPeak& a ){
                    return a.back.second + b;
                }) / pklist.second.size();
            double sd = std::accumulate(
                pklist.second.begin(), pklist.second.end(), 0.0, [&]( double b, const CountingPeak& a ){
                    return ( a.back.second - mean ) * ( a.back.second - mean );
                }) / ( pklist.second.size() - 1 );
            back.count = pklist.second.size();
            back.mean = mean;
            back.sd = sd;                        
        } while (0);
        ////////
        
        of << boost::format( "%.9le,\t%d" )
            % pklist.first // tof
            % pklist.second.size(); // count
        of << boost::format( "\t%9.5lf, %9.5lf" ) % apex.mean % apex.sd;
        of << boost::format( "\t%9.5lf, %9.5lf" ) % height.mean % height.sd;
        of << boost::format( "\t%9.5lf, %9.5lf" ) % front.mean % front.sd;
        of << boost::format( "\t%9.5lf, %9.5lf" ) % back.mean % back.sd
           << std::endl;
    }

    // boost::filesystem::path plt( file );
    // plt.replace_extension( ".plt" );
    // std::ofstream pf( plt.string() );
    // pf << "set terminal x11" << std::endl;
    // pf << "plot \"" << file << "\" using 1:2" << std::endl;
}
