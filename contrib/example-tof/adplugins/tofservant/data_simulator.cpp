/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "data_simulator.hpp"
#include <boost/random.hpp>
#include <boost/math/distributions/normal.hpp>
#include <boost/foreach.hpp>

using namespace tofservant;

boost::posix_time::ptime data_simulator::uptime_ = boost::posix_time::microsec_clock::local_time();
const double fScale = 1.5;
static const size_t nmasses = 10;

class waveform {
public:
    waveform();
    static double intensity( double mass, double sd );
    static void generate_spectrum( double sd, std::array< double, data_simulator::ndata >& );
    static void generate_trace( double sd, std::array< double, 256 >& );
};

struct molecule {
    const char * common_name_;
    const char * formula_;
    double abandance_;
    struct ma { double first; double second; } masses_[ nmasses ];
};

static molecule molecules [] = {
    { "4-diminopyridine", "C5H6N2", 1000, {
        { 94.0531,  100 }
        , { 95.05013,  0.7 }
        , { 95.05646,  5.4 }
        , { 0, 0 } }
    }
    , { "Methamidophos", "C2H8NO2PS", 500, {
              { 141.00134, 100 }
              , { 141.99838,  0.4 }
              , { 142.00073,  0.8 }
              , { 142.0047,   2.2 }
              , { 142.99714,  4.4 }
              , { 143.00558,  0.4 }
              , { 0, 0 } }
    }
    , { "Simazine", "C7H12ClN5", 250, {
                { 201.07812, 100 }
              , { 202.07516,  1.8 }
              , { 202.08148,  7.6 }
              , { 202.0844,   0.2 }
              , { 203.07517,  32.4 }
              , { 203.08483,  0.2 }
              , { 204.07221,  0.6 }
              , { 204.07853,  2.5 }
              , { 0, 0 } }
    }
    , { "Carbaryl", "C12H11NO2", 125, {
                { 201.07899,  100 }
              , { 202.07603,  0.4 }
              , { 202.08235,  13  }
              , { 202.08526,  0.2 }
              , { 203.08323,  0.4 }
              , { 203.0857,   0.8 }
              , { 0, 0 } }
    }
    , { "Pymetrozine", "C10H11N5O", 62, {
                { 217.09637, 100 }
              , { 218.09341,  1.8 }
              , { 218.09973, 10.8 }
              , { 218.10264,  0.2 }
              , { 219.09677, 0.2 }
              , { 219.10061, 0.2 }
              , { 219.10308, 0.5 }
              , { 0, 0 }  }
    }
};

static const size_t nmols = sizeof( molecules ) / sizeof ( molecules[ 0 ] );

double
waveform::intensity( double mz, double sd )
{
    double d = 0;

    for ( size_t i = 0; i < nmols; ++i ) {
        const molecule& m = molecules[ i ];

        for ( size_t k = 0; k < nmasses; ++k ) {
            const molecule::ma& ion = m.masses_[ k ];
            if ( ion.first < 0.5 )  // m/z 0.5 is not possible
                break;

            // normal distribution nd( mean, sd )
            boost::math::normal_distribution< double > nd( ion.first, sd );
            d += boost::math::pdf( nd, mz ) * ( m.abandance_ * ( ion.second / 100.0 ) );

        }
    }
    return d;
}

void
waveform::generate_spectrum( double sd, std::array< double, data_simulator::ndata >& intensities )
{
    for ( size_t idx = 0; idx < intensities.size(); ++idx ) {
        double mz = data_simulator::index_to_mass( idx );
        intensities[ idx ] = intensity( mz, sd );
    }
}

void
waveform::generate_trace( double sd, std::array< double, 256 >& intensities )
{
    for ( size_t idx = 0; idx < intensities.size(); ++idx ) {
        boost::math::normal_distribution< double > nd( 127.0, sd );
        intensities[ idx ] = boost::math::pdf( nd, idx ) * 10 + 0.05;
    }
}

///-------------------------

data_simulator::data_simulator() : peakwidth_( 0.16 )
{
    std::cout << "data_simulator::generate_spectrum..." << std::endl;
    waveform::generate_spectrum( peakwidth_, rawSpectrum_ );
    std::cout << "data_simulator::generate_trace..." << std::endl;
    waveform::generate_trace( 25.6, trace_ );
    std::cout << "data_simulator ctor done." << std::endl;
}

double
data_simulator::index_to_mass( size_t idx )
{
    if ( idx >= ndata )
        return 512.0;
    return idx * ( double(512) / ndata );
}

size_t
data_simulator::mass_to_index( double mass )
{
    if ( mass > 512.0 )
        return ndata - 1;
    if ( mass < 0 )
        return 0;
    return size_t( mass / ( double(512.0) * ndata ) );
}

void
data_simulator::generate_spectrum( size_t nAverage )
{
    boost::mt19937 gen( uint32_t( time(0) ) );
    boost::uniform_real<> distrib( -fScale, fScale * 2 );

    std::fill( intensities_.begin(), intensities_.end(), 0 );

    using namespace boost;
    boost::variate_generator< mt19937&, uniform_real<> > rand( gen, distrib );

    static int npos = 0;
    npos++;

    double sf = trace_[ npos % trace_.size() ] * 1000.0;
    while ( nAverage-- ) {
        for ( size_t i = 0; i < ndata; ++i ) {
            double a = double( rand() ) * fScale + rawSpectrum_[ i ] * sf;
            intensities_[ i ] += int32_t( a );
        }
    }

}

void
data_simulator::peakwidth( double w )
{
    if ( w >= 0.01 ) {
        peakwidth_ = w;
        waveform::generate_spectrum( peakwidth_, rawSpectrum_ );
    }
}

double
data_simulator::peakwidth() const 
{
    return peakwidth_;
}
