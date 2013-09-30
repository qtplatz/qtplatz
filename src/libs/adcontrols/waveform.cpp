/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "waveform.hpp"
#include "massspectrum.hpp"
#include "msproperty.hpp"
#include <adportable/array_wrapper.hpp>
#include <adportable/fft.hpp>
#include <vector>
#include <complex>

using namespace adcontrols;

waveform::waveform()
{
}

bool
waveform::fft::lowpass_filter( adcontrols::MassSpectrum& ms, double freq )
{
    if ( ms.isCentroid() )
        return false;

    size_t totalSize = ms.size();
	(void)totalSize;
	size_t N = 32;
    while ( N < ms.size() )
		N *= 2;
	const size_t NN = ms.size();
    unsigned long sampIntval = ms.getMSProperty().instSamplingInterval(); // ps
    if ( sampIntval == 0 )
        sampIntval = static_cast< unsigned long >( ( ( ms.getTime( ms.size() - 1, metric::pico ) - ms.getTime( 0, metric::pico ) ) / ms.size() ));

    const double T = N * double(sampIntval) * 1.0e-12;  // time full scale in seconds.  Freq = n/T (Hz)
    // power spectrum has N/2 points and is n/T Hz horizontal axis  := data[N/2] = (N/2)/T Hz
    size_t cutoff = size_t( T * freq );

	adportable::array_wrapper<const double> pIntens( ms.getIntensityArray(), N );

	std::vector< std::complex<double> > spc( N );
	std::vector< std::complex<double> > fft( N );
	size_t n;
	for ( n = 0; n < N && n < NN; ++n )
		spc[ n ] = std::complex<double>( pIntens[ n ] );
	while ( n < N )
		spc[ n++ ] = pIntens[ NN - 1 ];

	adportable::fft::fourier_transform( fft, spc, false );
    // appodization
    for ( size_t i = cutoff; i < N - cutoff; ++i )
        fft[ i ] = 0;
    //adportable::fft::apodization( N/2 - N/16, N / 16, fft );
	adportable::fft::fourier_transform( spc, fft, true );

	std::vector<double> data( N );
	for ( size_t i = 0; i < NN; ++i )
		data[ i ] = spc[i].real();

	ms.setIntensityArray( &data[0] );

	return true;
}

