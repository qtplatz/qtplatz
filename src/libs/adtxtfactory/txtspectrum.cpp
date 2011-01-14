// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "txtspectrum.h"
#include <adportable/string.h>
#include <adcontrols/description.h>
#include <adcontrols/massspectrum.h>
#include <adcontrols/msproperty.h>
#include <adcontrols/mscalibration.h>
#include <fstream>
#include <algorithm>

using namespace adtxtfactory;

TXTSpectrum::TXTSpectrum()
{
}

bool
TXTSpectrum::load( const std::wstring& name )
{
    std::string filename = adportable::string::convert( name );

    std::ifstream in( filename.c_str() );
    if ( in.fail() )
        return false;

    std::vector<double> timeArray, massArray, intensArray;

    double time, mass, intens;
    do {
        in >> time;
        in >> mass;
        in >> intens;
        timeArray.push_back( time );
        massArray.push_back( mass );
        intensArray.push_back( intens );
    } while( ! in.eof() );

    size_t size = timeArray.size();

    ms_.addDescription( adcontrols::Description( L"", name ) );

    adcontrols::MSProperty prop;

    const double t1 = timeArray.front();
    const double t2 = timeArray.back();
    const double m1 = massArray.front();
    const double m2 = massArray.back();

    double x = (t2 - t1) / size;
    unsigned long sampInterval = static_cast<unsigned long>( ( x * 1e12 ) + 0.5 ); // sec -> psec
    unsigned long startDelay = static_cast<unsigned long>( ( t1 * 1.0e12 ) / sampInterval + 0.5 );
    
    for ( size_t i = 0; i < size; ++i ) {
        double t = ( (startDelay * sampInterval) + (i * sampInterval) ) * 1e-12;
        double d = std::abs( timeArray[i] - t );
        assert( d < 1.0e-9 );
    }

    ms_.resize( size );
    // profile data does not require time array
    // ms_.setTimeArray( &timeArray[0] );
    ms_.setIntensityArray( &intensArray[0] );
    ms_.setAcquisitionMassRange( massArray.front(), massArray.back() );

    // theoretical calibration  [ sqrt(m) = a + b*t ]
    double b = ( std::sqrt( m2 ) - std::sqrt( m1 ) ) / ( t2 - t1 );
    double a = std::sqrt( m1 ) - b * t1;
    std::vector< double > coeffs;
    coeffs.push_back( a );
    coeffs.push_back( b );

    adcontrols::MSCalibration calib;
    calib.coeffs( coeffs );

    for ( size_t i = 0; i < massArray.size(); ++i ) {
        double m_sqrt = a + b * timeArray[i];
        double mz = m_sqrt * m_sqrt;
        double delta = massArray[i] - mz;
        // std::cout << massArray[i] << ", " << mz << " delta= " << delta << std::endl;
        massArray[i] = mz;
    }

    ms_.setMassArray( &massArray[0] );
    prop.setInstSamplingInterval( sampInterval );
    prop.setNumAverage(1);
    prop.setInstMassRange( ms_.getAcquisitionMassRange() );
    prop.setInstSamplingStartDelay( startDelay );
    ms_.setMSProperty( prop );
    ms_.setCalibration( coeffs );

	minValue_ = *std::min_element( intensArray.begin(), intensArray.end() );
	maxValue_ = *std::max_element( intensArray.begin(), intensArray.end() );

    return true;
}