// gcc -std=c++11 -o filter filter.cpp -lstdc++

#include <iostream>
#include <cstdint>
#include <cmath>
#include <vector>
#include <iomanip>
#include <random>
#include "../../src/libs/adportable/sgfilter.hpp"

#if defined __GNUC__
# pragma GCC diagnostic ignored "-Wdeprecated-register"
#endif
#include <boost/math/distributions/normal.hpp>

struct noise {

    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<> dist;

    noise( double min, double max ) : gen( rd() ), dist( min, max ) {}
    double operator()() {
        return dist( gen );
    };

};

const int M = 13;
const size_t N = 512;

int
main( int ac, char * av[] )
{
    std::vector< double > smooth, derivative;
    noise noise( 0.0, 0.05 );

    adportable::SGFilter smoother( M, adportable::SGFilter::Smoothing );
    adportable::SGFilter d1cubic( M, adportable::SGFilter::Derivative1 );
    adportable::SGFilter d1quadratic( M, adportable::SGFilter::Derivative1, adportable::SGFilter::Quadratic );

    std::vector<double> waveform;
    double mean = double(N) / 2;
    for ( int i = 0; i < N; ++i ) {
        boost::math::normal_distribution< double > nd( mean, double(N)/16 /* sd */);
        double y = boost::math::pdf( nd, i ) / boost::math::pdf( nd, mean ) + noise();
        waveform.push_back( y );
    }

    double a = waveform[0];
    for ( int i = M / 2; i < N - M; ++i ) {
        double s = smoother( waveform.data() + i );
        double dc = d1cubic( waveform.data() + i );
        double dq = d1quadratic( waveform.data() + i );

        std::cout << i << "\t" << std::fixed << std::setprecision(7)
                  << waveform[i] << "\t" << s 
                  << "\t" << dc
                  << "\t" << dq
                  << std::endl;
    }
}
