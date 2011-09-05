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

#include "txtspectrum.hpp"
#include <adportable/string.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adportable/debug.hpp>
#include <fstream>
#include <algorithm>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#if defined _MSC_VER
# pragma warning (disable: 4819)
#endif
#include <boost/numeric/interval.hpp>

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

    typedef boost::char_separator<char> separator;
    typedef boost::tokenizer< separator > tokenizer;

    separator sep( ", \t", "", boost::drop_empty_tokens );

    std::vector<double> timeArray, massArray, intensArray;

    do {
        double values[3];
        std::string line;
        if ( getline( in, line ) ) {

            tokenizer tokens( line, sep );
            int i = 0;
            for ( tokenizer::iterator it = tokens.begin(); it != tokens.end() && i < 3; ++it, ++i ) {
                std::string s = *it;
                values[i] = boost::lexical_cast< double >( s );
            }
            if ( i == 3 ) {
                timeArray.push_back( values[ 0 ] );
                massArray.push_back( values[ 1 ] );
                intensArray.push_back( values[ 2 ] );
            }
        }
    } while( ! in.eof() );

    size_t size = timeArray.size();

    ms_.addDescription( adcontrols::Description( L"", name ) );

    using adcontrols::MSProperty;
    MSProperty prop;
    std::vector<MSProperty::SamplingInfo> segments;

    const double t0 = timeArray.front();
    const double tt = timeArray.back();
    const double m0 = massArray.front();
    const double mt = massArray.back();

    double x = timeArray[1] - timeArray[0];
    unsigned long sampInterval = static_cast<unsigned long>( x * 1e12 + 0.5 ); // s -> psec
    const unsigned long startDelay = static_cast<unsigned long>(  ( t0 * 1e12 /*s*/) / sampInterval + 0.5 );

    do {
        unsigned long nDelay = startDelay;
        size_t nCount = 0;
        for ( std::vector<double>::const_iterator it = timeArray.begin() + 1; it != timeArray.end(); ++it ) {
            ++nCount;
            double x = it[ 0 ] - it[ -1 ];
            unsigned long interval = unsigned long( x * 1e12 + 0.5 ); // s --> ps
            if ( interval > sampInterval ) {
                segments.push_back( MSProperty::SamplingInfo(sampInterval, nDelay, nCount, 1 ) );
                if ( it + 1 != timeArray.end() ) {
                    sampInterval = unsigned long ( ( it[ 1 ] - it[ 0 ] ) * 1e12 + 0.5 );
                    nCount = 0;
                    nDelay = unsigned long( ( it[ 0 ] * 1e12 ) / sampInterval + 0.5 );
                }
            }
        }
        segments.push_back( MSProperty::SamplingInfo(sampInterval, nDelay, nCount, 1 ) );
    } while ( 0 );

    // double x = (t2 - t1) / size;

    // validation
    unsigned long tolerance = sampInterval / 10.0;
    std::vector< MSProperty::SamplingInfo >::const_iterator sampInfo = segments.begin();
    // size_t nDelay = sampInfo->nSamplingDelay;
    for ( size_t i = 0, k = 0; i < size; ++i, ++k ) {
        if ( k == sampInfo->nSamples ) {
            if ( ++sampInfo == segments.end() ) {
                if ( i + 1 < size )
                    adportable::debug() << "text file loader: time/segment validation failed at idx " << int( i );
                break;
            }
            k = 0;
        }
        unsigned long t1 = MSProperty::toSeconds( i, segments ) * 1e12 + 0.5;
        unsigned long t2 = ( sampInfo->nSamplingDelay + k ) * sampInfo->sampInterval;
        if ( t1 != t2 )
            adportable::debug() << "text file loader: library calculation error at " << int(i) << " time: " << timeArray[i] << "(s) expected: " << t1 * 1e-12;
        long error = long( timeArray[i] * 1e12 + 0.5 ) - t1;
        if ( std::abs( error ) >= tolerance ) 
            adportable::debug() << "text file loader: time distance error at " << int(i) << " time: " << timeArray[i] * 1e6<< "(us) error: " << error << "ps";
    }

    ms_.resize( size );
    // profile data does not require time array
    // ms_.setTimeArray( &timeArray[0] );
    ms_.setIntensityArray( &intensArray[0] );
    ms_.setAcquisitionMassRange( massArray.front(), massArray.back() );

    // theoretical calibration  [ sqrt(m) = a + b*t ]
    double b = ( std::sqrt( mt ) - std::sqrt( m0 ) ) / ( tt - t0 );
    double a = std::sqrt( m0 ) - b * t0;
    std::vector< double > coeffs;
    coeffs.push_back( a );
    coeffs.push_back( b );

    adcontrols::MSCalibration calib;
    calib.coeffs( coeffs );

    std::pair< double, double > error;
    for ( size_t i = 0; i < massArray.size(); ++i ) {
        double m_sqrt = adcontrols::MSCalibration::compute( calib.coeffs(), timeArray[i] );
        double mz = m_sqrt * m_sqrt;
        double delta = ( massArray[i] - mz ) * 1000; // mDa
        if ( delta < error.first )
            error.first = delta;
        if ( delta > error.second )
            error.second = delta;
        if ( std::abs( delta ) > 1e-9 )
            adportable::debug() << massArray[i] << ", " << mz << " delta= " << delta << "mDa";
    }
    adportable::debug() << "calibration error: " << error.first << ", " << error.second << "mDa";

    ms_.setMassArray( &massArray[0] );
    prop.setInstSamplingInterval( sampInterval );
    prop.setNumAverage(1);
    prop.setInstMassRange( ms_.getAcquisitionMassRange() );
    prop.setInstSamplingStartDelay( startDelay );
    prop.setSamplingInfo( segments );
    ms_.setMSProperty( prop );
    ms_.setCalibration( coeffs );

	minValue_ = *std::min_element( intensArray.begin(), intensArray.end() );
	maxValue_ = *std::max_element( intensArray.begin(), intensArray.end() );

    return true;
}
