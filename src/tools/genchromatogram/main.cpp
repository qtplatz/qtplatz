/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <boost/program_options.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

namespace po = boost::program_options;

using peak = std::tuple< double, double, double >;  // tR, intensity, sigma
enum { tR = 0, intensity = 1, sigma = 2 };

struct noise {
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<> dist;

    noise( double min, double max ) : gen( rd() ), dist( min, max ) {}
    double operator()() {  return dist( gen );  }
};

struct Resolution {
    const size_t N_;
    const double sqrN_;
    Resolution( size_t N ) : N_( N ), sqrN_(std::sqrt(double(N))) {};
    double retentionTime( double tR1, double Rs ) const {
        return (sqrN_ + 2*Rs) * tR1 / (sqrN_ - 2 * Rs);
    }
};

// y(t) = A * exp( -( (t - tR)^2)/2*sigma^2)

class Chromatogram {
public:
    Chromatogram( size_t N
                  , double sampIntval ) : N_( N )
                                        , samplingInterval_( sampIntval ) {
    }

    void addPeak( double tR, double h ) {
        double sigma = tR / std::sqrt(static_cast< double >(N_));
        auto it = std::lower_bound( peaks_.begin(), peaks_.end(), tR
                                    , [](const auto& a, const auto& b){ return std::get<0>(a) < b; });
        peaks_.emplace( it, tR, h, sigma );
    }

    // operator
    std::vector< std::pair< double, double > >
    operator ()( double tstart = 0, double tstop = 0 ) const {

        if ( tstop == 0 )
            tstop = std::get<tR>(peaks_.back() ) +  5 * std::get<sigma>(peaks_.back() );

        std::vector< std::pair< double, double > > chro;
        double t = tstart;
        while ( t <= tstop ) {
            double y = 0;
            std::for_each( peaks_.begin(), peaks_.end(), [&](const auto& pk ){ y += at( t, pk ); });
            chro.emplace_back( t, y );
            t = chro.size() * samplingInterval_;
        }
        return chro;
    }
    const std::vector< peak > peaks() const { return peaks_; }

private:
    constexpr double at( double t, const peak& pk ) const {
        double exponent = - std::pow(t - std::get<tR>(pk), 2) / (2 * std::pow( std::get<sigma>(pk), 2 ) );
        return std::get<intensity>(pk) * std::exp(exponent);
    }

    size_t N_;
    double samplingInterval_;
    std::vector< peak > peaks_;
};

int
main(int argc, char *argv[])
{
    po::variables_map vm;
    po::options_description description( argv[0] );
    {
        description.add_options()
            ( "help,h",      "Display this help message" )
            ( "ntp,N",   po::value< size_t >()->default_value( 8'000 ) )
            ( "noise",   po::value< double >(), "noise amplitude." )
            ( "drift",   po::value< double >(), "intensity per second, ex. 0.01" )
            ( "resolution", po::value< double >() )  //  2.0*(tR2-tR1)/(W1+W2)
            ;
        po::store( po::command_line_parser( argc, argv ).options( description ).run(), vm );
        po::notify(vm);
    }

    if ( vm.count( "help" ) ) {
        std::cerr << description << std::endl;
        return 0;
    }

    const size_t N = vm["ntp"].as<size_t>(); // 8'000;

    Chromatogram chro{ N, 0.15 };

    // generate t0 noise & peak
    chro.addPeak(  10.0  - 10.0 / (2*std::sqrt(10000)), -50 );
    chro.addPeak(  10.0, 50 );

    // retention time should not match any of data sampling point
    chro.addPeak( 15.33, 1000 );

    if ( vm.count( "resolution" ) ) {
        chro.addPeak( Resolution(N).retentionTime( 15.33, vm["resolution"].as<double>() ), 800 );
    } else {
        chro.addPeak( 20.78, 800 );
    }

    auto trace = chro(0,30);

    for ( const auto [tR,H,sigma]: chro.peaks() )
        std::cerr << std::format("{}\t{}\t{}", tR, H, sigma*4) << std::endl;

    if ( vm.count( "noise" ) ) {
        noise noise( -vm["noise"].as<double>()/2, vm["noise"].as<double>()/2 );
        std::for_each( trace.begin(), trace.end(), [&](auto& d){ get<1>(d) += noise(); }) ;
    }

    for ( const auto& data: trace )
        std::cout << std::format("{:.4}\t{}", std::get<0>(data), int(std::get<1>(data) + 50)) << std::endl;
}
