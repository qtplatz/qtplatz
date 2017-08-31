/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "waveform_filter.hpp"
#include "massspectrum.hpp"
#include "msproperty.hpp"
#include "samplinginfo.hpp"
#include <adportable/array_wrapper.hpp>
#include <adportable/debug.hpp>
#include <adportable/fft.hpp>
#include <adportable/float.hpp>
#include <adportable/sgfilter.hpp>
#include <algorithm>
#include <complex>
#include <ratio>
#include <vector>

extern "C" {
    // Takuya OOURA's package, built on adportable static library
    void cdft( int, int isgn, double * );
    void rdft( int, int isgn, double * );    
}

namespace adcontrols {
    namespace waveform_filter_helper {

        struct transform {

            template< typename T >            
            size_t operator()( const double * p, size_t size, std::vector< T >& d ) const {

                if ( size < 32 )
                    return 0;

                size_t N = 32;
                while ( N < size )
                    N *= 2;

                d.resize( N );
                std::copy( p, p + size, d.begin() );
                std::fill( d.begin() + size, d.end(), p[ size - 1 ] );

                return N;
            }
        };

        struct appodization {
            template< typename T >
            void operator()( std::vector< T >& a, const std::pair< size_t, size_t >& cutoff ) const {

                // high-pass
                if ( cutoff.first ) {
                    // triangle
                    auto llimit = cutoff.first / 2;
                    for ( size_t i = 0; i < llimit; ++i ) {
                        double factor = double(i) / llimit;
                        a[ i + 1 ] *= factor; // Save DC component 
                        a[ a.size() - i - 1 ] *= double( i / llimit );
                    }
                    // square 
                    //std::fill( a.begin() + 1,                      a.begin() + 1 + llimit,    0 ); // keep DC component
                    //std::fill( a.begin() + ( a.size() - llimit ),  a.end(),        0 );
                }

                // low-pass
                if ( cutoff.second && a.size() > cutoff.second * 2)
                    std::fill( a.begin() + cutoff.second, a.begin() + ( a.size() - cutoff.second ), 0 );
            }
        };

        struct zerofilling {
            template< typename T >
            const std::vector< T > operator()( const std::vector< T >& a, size_t NN ) const {
                
                std::vector< T > b( NN );
                std::fill( b.begin(), b.end(), 0 );

                size_t sz = a.size() / 2;

                std::copy( a.begin(), a.begin() + sz + 1, b.begin() );
                std::copy( a.begin() + sz + 1, a.begin() + a.size(), b.begin() + ( NN - sz ) );

                return b;
            }
        };
        

        struct helper {

            static double sampInterval( const adcontrols::MassSpectrum& ms ) {
                double interval = ms.getMSProperty().samplingInfo().fSampInterval(); // seconds
                if ( interval < ( 1.0 / std::pico::den ) ) // 1ps
                    interval = ( ms.getTime( ms.size() - 1 ) - ms.getTime( 0 ) ) / ms.size(); // average
                return interval;
            }
            
            static std::pair< size_t, size_t > cutoffIndex( double sampInterval, size_t N, double h_freq, double l_freq ) {
                return std::make_pair(
                    size_t( ( N * sampInterval ) * l_freq )
                    , size_t( ( N * sampInterval ) * h_freq )
                    );
            }
        };
        
        
    }
}

using namespace adcontrols;
using namespace adcontrols::waveform_filter_helper;

waveform_filter::waveform_filter()
{
}

bool
waveform_filter::fft::lowpass_filter( adcontrols::MassSpectrum& ms, double freq )
{
    return bandpass_filter( ms, freq, 0 );
}

bool
waveform_filter::fft::lowpass_filter( std::vector<double>& intens, double sampInterval, double freq )
{
    return bandpass_filter( intens, sampInterval, freq, 0 );
}

bool
waveform_filter::fft::bandpass_filter( adcontrols::MassSpectrum& ms, double hfreq, double lfreq )
{
    std::vector< std::complex< double > > spc;

    if ( const auto N = transform()( ms.getIntensityArray(), ms.size(), spc ) ) {

        const double sampInterval = helper::sampInterval( ms );
        std::vector< std::complex<double> > fft( N );        

        adportable::fft::fourier_transform( fft, spc, false );

        appodization()( fft, helper::cutoffIndex( sampInterval, N, hfreq, lfreq ) );

        adportable::fft::fourier_transform( spc, fft, true );

        for ( size_t i = 0; i < ms.size(); ++i )
            ms.setIntensity( i, spc[i].real() );
        
        return true;
    }
    return false;
}

bool
waveform_filter::fft::bandpass_filter( std::vector<double>& intens, double sampInterval, double hfreq, double lfreq )
{
    std::vector< std::complex< double > > spc;

    if ( const size_t N = transform()( intens.data(), intens.size(), spc ) ) {

        std::vector< std::complex< double > > fft( N );
        adportable::fft::fourier_transform( fft, spc, false );
        
        appodization()( fft, helper::cutoffIndex( sampInterval, N, hfreq, lfreq ) );

        adportable::fft::fourier_transform( spc, fft, true );

        auto src = spc.begin();
    
        for ( auto it = intens.begin(); it != intens.end(); ++it ) {
            *it = src->real();
            ++src;
        }
        
        return true;
    }
    return false;
}

bool
waveform_filter::fft4g::lowpass_filter( adcontrols::MassSpectrum& ms, double freq )
{
    return bandpass_filter( ms, freq );
}

bool
waveform_filter::fft4g::lowpass_filter( size_t size, double * data, double sampInterval, double freq )
{
    return bandpass_filter( size, data, sampInterval, freq, 0 );
}

//////////////////////
bool
waveform_filter::fft4c::lowpass_filter( adcontrols::MassSpectrum& ms, double freq )
{
    return bandpass_filter( ms, freq, 0 );
}

bool
waveform_filter::fft4c::lowpass_filter( size_t size, double * data, double sampInterval, double freq )
{
    return bandpass_filter( size, data, sampInterval, freq, 0 );
}

bool
waveform_filter::fft4g::bandpass_filter( adcontrols::MassSpectrum& ms, double h_freq, double l_freq )
{
	std::vector< double > a;

	const double sampInterval = helper::sampInterval( ms );
    
    if ( const size_t N = transform()( ms.getIntensityArray(), ms.size(), a ) ) {

        rdft( N, 1, a.data() );

        appodization()( a, helper::cutoffIndex( sampInterval, N, h_freq, l_freq ) );

        rdft( N, -1, a.data() );

        std::transform( a.begin(), a.begin() + ms.size(), a.begin(), [N] ( double a ) { return a * 2.0 / N; } );
        ms.setIntensityArray( a.data() );
        return true;
    }
    
    return false;
}

bool
waveform_filter::fft4g::bandpass_filter( size_t size, double * data, double sampInterval, double h_freq, double l_freq )
{
	std::vector< double > a;
    
    if ( const size_t N = transform()( data, size, a ) ) {

        rdft( N, 1, reinterpret_cast<double *>( a.data() ) );    

        appodization()( a, helper::cutoffIndex( sampInterval, N, h_freq, l_freq ) );

        rdft( N, -1, reinterpret_cast<double *>( a.data() ) );

        auto src = a.begin();

        for ( double * it = data; it != data + size; ++it, ++src )
            *it = *src * 2.0 / N;

        return true;
    }

	return false;
}

bool
waveform_filter::fft4c::bandpass_filter( adcontrols::MassSpectrum& ms, double h_freq, double l_freq )
{
	std::vector< std::complex<double> > a;

	double sampInterval = ms.getMSProperty().samplingInfo().fSampInterval(); // seconds
    if ( sampInterval == 0 )
        sampInterval = ( ms.getTime( ms.size() - 1 ) - ms.getTime( 0 ) ) / ms.size();

    if ( const size_t N = transform()( ms.getIntensityArray(), ms.size(), a ) ) {

        cdft( N * 2, 1, reinterpret_cast<double *>( a.data() ) );

        auto cutoff = helper::cutoffIndex( sampInterval, N, h_freq, l_freq );

        appodization()( a, cutoff );

        cdft( N * 2, -1, reinterpret_cast<double *>( a.data() ) );

        for ( size_t i = 0; i < ms.size(); ++i )
            ms.setIntensity( i, a[ i ].real() * 2.0 / ( N * 2 ) );

        return true;
    }
    
    return false;
}

bool
waveform_filter::fft4c::bandpass_filter( size_t size, double * data, double sampInterval, double h_freq, double l_freq )
{
	std::vector< std::complex<double> > a;
    
    if ( const size_t N = transform()( data, size, a ) ) {

        cdft( N * 2, 1, reinterpret_cast<double *>( a.data() ) );    
        
        appodization()( a, helper::cutoffIndex( sampInterval, N, h_freq, l_freq ) );

        cdft( N * 2, -1, reinterpret_cast<double *>( a.data() ) );

        auto src = a.begin();

        for ( double * it = data; it != data + size; ++it, ++src )
            *it = src->real() * 2.0 / ( N * 2 );

        return true;
    }
    return false;
}

bool
waveform_filter::fft4c::zero_filling( adcontrols::MassSpectrum& ms
                                      , double freq
                                      , std::function< double( double ) > assign_mass )
{
	double sampInterval = ms.getMSProperty().samplingInfo().fSampInterval(); // seconds
    if ( sampInterval == 0 )
        sampInterval = ( ms.getTime( ms.size() - 1 ) - ms.getTime( 0 ) ) / ms.size();

	std::vector< std::complex<double> > t;
    
    if ( const size_t N = transform()( ms.getIntensityArray(), ms.size(), t ) ) {

        cdft( N * 2, 1, reinterpret_cast<double *>( t.data() ) );

        const double T = t.size() * sampInterval;
        double nyquist = ( t.size() / 2 ) / T;
        double dc = t[ 0 ].real();
        
        ADDEBUG() << "nyquist = " << nyquist * 1e-6 << "MHz, dc=" << dc;

        size_t fold( 1 );
        while ( ( nyquist * fold ) < freq && fold < 4 )
            fold *= 2;

        size_t NN = N * fold;
        
        if ( NN > N ) {
            std::vector< std::complex< double > > a = zerofilling()( t, NN );
            
            cdft( NN * 2, -1, reinterpret_cast<double *>( a.data() ) );

            ms.resize( NN );
            sampInterval /= fold;
            
            if ( ms.getMSProperty().samplingInfo().fSampInterval() > 0 ) {
                auto sampInfo = ms.getMSProperty().samplingInfo();
                sampInfo.setSampInterval( sampInterval );
                ms.getMSProperty().setSamplingInfo ( sampInfo );
            }

            const bool hasTime = ms.getTimeArray();
            const double t0 = ms.getTime( 0 );
            
            for ( size_t i = 0; i < ms.size(); ++i ) {
                ms.setIntensity( i, a[ i ].real() * 2.0 / ( NN * 2 ) );
                if ( hasTime )
                    ms.setTime( i, t0 + i * sampInterval );
                ms.setMass( i, assign_mass( t0 + i * sampInterval ) );
            }

            return true;
        }
    }
    return false;
}


/////////////////////

bool
waveform_filter::sg::lowpass_filter( adcontrols::MassSpectrum& ms, double width )
{
    double sampInterval = ms.getMSProperty().samplingInfo().fSampInterval(); // seconds
    size_t npts = uint32_t( width / sampInterval ) & 01;

    if ( npts < 3 || ms.isCentroid() )
        return false;

    using adportable::SGFilter;

    SGFilter smoother( int( npts ), SGFilter::Smoothing, SGFilter::Cubic );
    
    std::vector< double > data( ms.size() );
    const double * pi = ms.getIntensityArray();
    
    for ( size_t i = npts / 2; i < ms.size() - ( npts / 2 ); ++i )
        data[i] = smoother( pi + i );

    for ( size_t i = npts / 2; i < ms.size() - ( npts / 2 ); ++i )
        ms.setIntensity( i, data[i] );

    return false;
}

bool
waveform_filter::sg::lowpass_filter( size_t size, double * data, double sampInterval, double width )
{
    size_t npts = unsigned ( width / sampInterval ) | 01;
    if ( npts < 5 )
        npts = 5;

    using adportable::SGFilter;

    SGFilter smoother( int( npts ), SGFilter::Smoothing, SGFilter::Cubic );

    std::vector< double > sdata( size );

    for ( size_t i = npts / 2; i < size - ( npts / 2 ); ++i )
        sdata[i] = smoother( data + i );

    std::copy( sdata.begin() + npts / 2, sdata.begin() + size - ( npts / 2 ), data + npts / 2 );

    return true;
}

