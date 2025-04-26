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

#include <algorithm>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <cmath>

namespace po = boost::program_options;

using peak = std::tuple< double, double, double >;  // tR, intensity, sigma
enum { tR = 0, intensity = 1, sigma = 2 };

// y(t) = A * exp( -( (t - tR)^2)/2*sigma^2)

class Chromatogram {
public:
    Chromatogram( size_t N, double sampIntval ) : N_( N )
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

        for ( const auto& pk: peaks_ )
            std::cerr << std::format("{}\t{}\t{}", std::get<0>(pk), std::get<1>(pk), std::get<2>(pk)*4) << std::endl;

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

private:
    double at( double t, const peak& pk ) const {
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
    const int N = 8'000;

    Chromatogram chro{ N, 0.15 };

    chro.addPeak(  10.0  - 10.0 / (2*std::sqrt(10000)), -50 );
    chro.addPeak(  10.0, 50 );

    chro.addPeak( 15.33, 1000 );
    chro.addPeak( 20.78, 800 );

    auto trace = chro(0,30);
    for ( const auto& data: trace )
        std::cout << std::format("{:.4}\t{}", std::get<0>(data), int(std::get<1>(data) + 50)) << std::endl;
}
