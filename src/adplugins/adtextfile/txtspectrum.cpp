// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include <compiler/disable_unused_parameter.h>

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
#include <boost/numeric/interval.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>


using namespace adtextfile;

TXTSpectrum::TXTSpectrum()
{
}

bool
TXTSpectrum::load( const std::wstring& name )
{
	bool hasMass( false );
	boost::filesystem::path path( name );

	boost::filesystem::ifstream in( path );
    if ( in.fail() ) 
        return false;

    typedef boost::char_separator<char> separator;
    typedef boost::tokenizer< separator > tokenizer;

    separator sep( ", \t", "", boost::drop_empty_tokens );

    std::vector<double> timeArray, massArray, intensArray;

	size_t numSamples = 0;
	size_t numTurns = 0;
	if ( path.extension() == ".csv" ) {
		std::string line;
		while ( getline( in, line ) ) {
			if ( line.find( "All Elements" ) != line.npos ) {
				getline( in, line );
				numSamples = atol( line.c_str() );
			} else if ( line.find( "Turn:" ) != line.npos ) {
				getline( in, line );
				numTurns = atol( line.c_str() );
			} else if ( line.find( "Data" ) != line.npos ) {
				break;
			}
		}
	}

    do {
		bool hasMass = false;
        double values[3];
        std::string line;
        if ( getline( in, line ) ) {
            tokenizer tokens( line, sep );

            int i = 0;
            for ( tokenizer::iterator it = tokens.begin(); it != tokens.end() && i < 3; ++it, ++i ) {
                const std::string& s = *it;
                values[i] = atof( s.c_str() ); // boost::lexical_cast<double> in gcc throw bad_cast for "9999" format.
            }
			if ( i == 2 ) {
				timeArray.push_back( values[ 0 ] );
				intensArray.push_back( values[ 1 ] );
			} else if ( i == 3 ) {
				hasMass = true;
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

    for ( size_t i = 1; i < timeArray.size(); ++i ) {
        if ( timeArray[ i - 1 ] > timeArray[ i ] )
            adportable::debug() << "sequence not ordered";
    }

    double x = timeArray[1] - timeArray[0];
    unsigned long sampInterval = static_cast<unsigned long>( x * 1e12 + 0.5 ); // s -> psec
	const unsigned long startDelay = static_cast<unsigned long>(  ( timeArray.front() * 1e12 /*s*/) / sampInterval + 0.5 );

    do {
        unsigned long nDelay = startDelay;
        size_t nCount = 0;
        for ( std::vector<double>::const_iterator it = timeArray.begin() + 1; it != timeArray.end(); ++it ) {
            ++nCount;
            double x = it[ 0 ] - it[ -1 ];
            unsigned long interval = static_cast<unsigned long>( x * 1e12 + 0.5 ); // s --> ps
            if ( interval > sampInterval ) {
                segments.push_back( MSProperty::SamplingInfo(sampInterval, nDelay, nCount, 1 ) );
                if ( it + 1 != timeArray.end() ) {
                    sampInterval = static_cast<unsigned long>( ( it[ 1 ] - it[ 0 ] ) * 1e12 + 0.5 );
                    nCount = 0;
                    nDelay = static_cast<unsigned long>( ( it[ 0 ] * 1e12 ) / sampInterval + 0.5 );
                }
            }
        }
        segments.push_back( MSProperty::SamplingInfo(sampInterval, nDelay, nCount + 1, 1 ) );
    } while ( 0 );

    // validation
    unsigned long tolerance = static_cast< unsigned long >( sampInterval / 10.0 + 0.5 );
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
        unsigned long t1 = static_cast< unsigned long >( MSProperty::toSeconds( i, segments ) * 1e12 + 0.5 );
        unsigned long t2 = ( sampInfo->nSamplingDelay + k ) * sampInfo->sampInterval;
        if ( t1 != t2 )
            adportable::debug() 
                << "text file loader: library calculation error at " 
                << int(i) << " time: " << timeArray[i] << "(s) expected: " << t1 * 1e-12;
        long error = long( timeArray[i] * 1e12 + 0.5 ) - t1;
        if ( unsigned( std::abs( error ) ) >= tolerance ) 
            adportable::debug() 
                << "text file loader: time distance error at " 
                << int(i) << " time: " << timeArray[i] * 1e6<< "(us) error: " << error << "ps";
    }
    prop.setInstSamplingInterval( sampInterval );
    prop.setNumAverage(1);
    prop.setInstMassRange( ms_.getAcquisitionMassRange() );
    prop.setInstSamplingStartDelay( startDelay );
    prop.setSamplingInfo( segments );

    ms_.setMSProperty( prop );
    ms_.resize( size );
    ms_.setIntensityArray( intensArray.data() );

    if ( hasMass ) {

        ms_.setMassArray( massArray.data() );
        ms_.setAcquisitionMassRange( massArray.front(), massArray.back() );

    } else {
        // todo: add UD dialog box to ask those values
        double t1 = timeArray.front();
        double t2 = timeArray.back();
        double m1 = 500;
        double m2 = 800;
        
        // theoretical calibration  [ sqrt(m) = a + b*t ]
        double b = ( std::sqrt( m2 ) - std::sqrt( m1 ) ) / ( t2 - t1 );
        double a = std::sqrt( m1 ) - b * t1;
        std::vector< double > coeffs;
        coeffs.push_back( a );
        coeffs.push_back( b );
        
        adcontrols::MSCalibration calib;
        calib.coeffs( coeffs );
        
		massArray.resize( size );
        std::pair< double, double > error;
        for ( size_t i = 0; i < size; ++i ) {
            double m_sqrt = adcontrols::MSCalibration::compute( calib.coeffs(), ms_.getTime( i ) );
            double mz = m_sqrt * m_sqrt;
            ms_.setMass( i, mz );
        }
        ms_.setCalibration( coeffs );
		ms_.setAcquisitionMassRange( ms_.getMass( 0 ), ms_.getMass( size - 1 ) );
    }

	minValue_ = *std::min_element( intensArray.begin(), intensArray.end() );
	maxValue_ = *std::max_element( intensArray.begin(), intensArray.end() );

    return true;
}
