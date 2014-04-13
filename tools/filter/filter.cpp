// gcc -std=c++11 -o filter filter.cpp -lstdc++
#include <iostream>
#include <cstdint>
#include <cmath>
#include <vector>
#include <iomanip>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/math/distributions/normal.hpp>
#include <boost/random.hpp>

class SGCubic {
public:

    static void smoothing_coefficients( boost::numeric::ublas::vector< double >& coeffs, int m ) {

        m |= 0x01; // should be odd
        int n = m / 2;
        double cd = ( ( m * (std::pow(m,2) - 4 ) ) / 3 );

        std::cout << "smooth size=" << m << std::endl;
        coeffs.resize( m );

        int k = 0;
        for ( int i = -n; i <= n; ++i ) {
            double cn = ( ( 3 * std::pow(m,2) - 7 - 20 * std::pow(i,2) ) / 4.0 );
            coeffs[k++] = cn / cd;
        }
    }

    static void derivative1_coefficients( boost::numeric::ublas::vector< double >& coeffs, int m ) {

        m |= 0x01; // should be odd
        int n = m / 2;
        coeffs.resize( m );

        const double m2 = std::pow(m, 2);
        const double m4 = std::pow(m, 4);

        double cd = m * (m2 - 1) * (3 * std::pow(m,4) - 39.0 * std::pow(m,2) + 108) / 15.0;
        int k = 0;
        for ( int i = -n; i <= n; ++i ) {
            double cn = ( 5 * 
                          ( 3 * std::pow(m, 4) - 18 * std::pow(m, 2) + 31 ) * i )
                - ( 28 * (3 * std::pow(m, 2) - 7) * std::pow(i,3) );
            coeffs[k++] = cn / cd;
        }
    }

    static inline double convolute( const double * py, const boost::numeric::ublas::vector<double>& coeffs ) {
        double fxi = 0;
        int m = coeffs.size();
        py -= m / 2;
        for ( int i = 0; i < m; ++i )
            fxi += py[ i ] * coeffs[ i ];
        return fxi;
    }

};

struct noise {

    boost::mt19937 gen;

    boost::uniform_real<> dist;
    noise( double min, double max ) : dist( min, max ) {}
    double operator()() {
        boost::variate_generator< boost::mt19937&, boost::uniform_real<> > rand( gen, dist );
        return rand();
    };

};

const int M = 100;
const size_t N = 512;

int
main( int ac, char * av[] )
{
    boost::numeric::ublas::vector< double > smooth, derivative;
    noise noise( -0.1, +0.1 );

    std::cout << "smooth:" << std::endl;
    SGCubic::smoothing_coefficients( smooth, M );

    std::cout << "derivative:" << std::endl;
    SGCubic::derivative1_coefficients( derivative, M );

    std::vector<double> waveform;
    double mean = double(N) / 2;
    for ( int i = 0; i < N; ++i ) {
        boost::math::normal_distribution< double > nd( mean, double(N)/32 /* sd */);
        double y = boost::math::pdf( nd, i ) / boost::math::pdf( nd, mean ) + noise();
        waveform.push_back( y );
    }
    
    for ( int i = M / 2; i < N - M / 2; ++i ) {
        double s = SGCubic::convolute( waveform.data() + i, smooth );
        double d = SGCubic::convolute( waveform.data() + i, derivative );
        std::cout << i << "\t" << std::fixed << std::setprecision(7) << waveform[i] << "\t" << s << "\t" << d << std::endl;
    }

}
