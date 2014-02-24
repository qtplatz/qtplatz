// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include <adportable/spectrum_processor.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adlog/logger.hpp>
#include <adportable/textfile.hpp>
#include <fstream>
#include <algorithm>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/interval.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/datainterpreter.hpp>

using namespace adportable;
using namespace adcontrols;
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

	std::vector< std::wstring > supported_models = adcontrols::MassSpectrometer::get_model_names();

	if ( path.extension() == ".csv" ) {
        for ( const auto& model: supported_models ) {

            if ( auto spectrometer = adcontrols::MassSpectrometer::find( model.c_str() ) ) {
                const adcontrols::DataInterpreter& interpreter = spectrometer->getDataInterpreter();

                auto ms = std::make_shared< adcontrols::MassSpectrum >();
				if ( interpreter.compile_header( *ms, in ) ) {
                    compiled_ = ms;
                    break;
                }
            }
        }
    }

    do {
        double values[3];
        std::string line;
        if ( textfile::getline( in, line ) ) {
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

    std::vector<MSProperty::SamplingInfo> segments;
    if ( analyze_segments( segments, timeArray, compiled_.get() ) )
        validate_segments( segments, timeArray );

    size_t idx = 0;
    size_t fcn = 0;
    for ( auto s: segments ) {
        std::shared_ptr< adcontrols::MassSpectrum > ptr( new adcontrols::MassSpectrum );
		if ( massArray.empty() )
			ptr->setAcquisitionMassRange( 100, 1000 );
		else
			ptr->setAcquisitionMassRange( massArray.front(), massArray.back() );
        idx += create_spectrum( *ptr, idx, s, timeArray, massArray, intensArray, fcn++ );
        spectra_.push_back( ptr );
    }
    return true;
}

bool
TXTSpectrum::analyze_segments( std::vector<adcontrols::MSProperty::SamplingInfo>& segments
                               , const std::vector<double>& timeArray
                               , const adcontrols::MassSpectrum * compiled )
{
    if ( timeArray.empty() )
        return false;

    // estimate sampling interval
    double x = timeArray[1] - timeArray[0];
    int sampInterval = static_cast<int>( x * 1e12 + 0.5 ); // s -> psec
	if ( sampInterval < 505 )
		sampInterval = 500;
	else if ( sampInterval < 1005 )
		sampInterval = 1000;

	const unsigned long startDelay = static_cast<unsigned long>(  ( timeArray.front() * 1e12 /*s*/) / sampInterval + 0.5 );

    unsigned long nDelay = startDelay;
    size_t nCount = 0;

    size_t idx = 0;
    for ( std::vector<double>::const_iterator it = timeArray.begin() + 1; it != timeArray.end(); ++it ) {
        ++nCount;
        ++idx;
        double x = it[ 0 ] - it[ -1 ];

        if ( ( x < 0 ) || ( x > double( sampInterval ) * 2.0e-12 ) ) {

			int mode = find_mode( segments.size() );
            segments.push_back( adcontrols::MSProperty::SamplingInfo(sampInterval, nDelay, nCount, 1, mode ) );

            if ( it + 1 != timeArray.end() ) {
                sampInterval = static_cast<unsigned long>( ( it[ 1 ] - it[ 0 ] ) * 1e12 + 0.5 );
                if ( sampInterval < 505 )
                    sampInterval = 500;
                else if (sampInterval < 1005 )
                    sampInterval = 1000;
                nCount = 0;
                double t0 = *it;
                nDelay = int( ( t0 / ( sampInterval * 1e-12 ) ) + 0.5 );
                ADTRACE()
                    << "time error: " << (t0 - ( nDelay * sampInterval * 1e-12 ) ) * 1e12 << "ps : t=" << t0  << " @ " << idx;
            }
        }
    }
	int mode = find_mode( segments.size() );
    segments.push_back( adcontrols::MSProperty::SamplingInfo(sampInterval, nDelay, nCount + 1, 1, mode ) );
    return true;
}

int
TXTSpectrum::find_mode( size_t idx ) const
{
	int mode = 0;
    if ( compiled_ ) {
		adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( *compiled_ );
		if ( idx < segs.size() )
			mode = segs[ idx ].getMSProperty().getSamplingInfo().mode;
    }
    return mode;
}

bool
TXTSpectrum::validate_segments( const std::vector<adcontrols::MSProperty::SamplingInfo>& segments, const std::vector<double>& timeArray )
{
	/*
    for ( size_t i = 0; i < timeArray.size(); ++i ) {

        double t = adcontrols::MSProperty::toSeconds( i, segments );
        if ( std::abs( t - timeArray[i] ) > 0.5e-9 ) {
            adcontrols::MSProperty::toSeconds( i, segments );
            adportable::debug(__FILE__, __LINE__) << "error: " << t - timeArray[i]
                                                  << "actual:" << timeArray[i] 
                                                  << " calculated:" << t;
        }
    }
	*/
    return true;
}

size_t
TXTSpectrum::create_spectrum( adcontrols::MassSpectrum& ms, size_t idx
                              , const adcontrols::MSProperty::SamplingInfo& info
                              , const std::vector<double>& timeArray
                              , const std::vector<double>& massArray
                              , const std::vector<double>& intensArray
                              , size_t fcn )
{
    MSProperty prop;

    if ( compiled_ ) {
        prop = compiled_->getMSProperty();
		adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( *compiled_ );
		if ( segments.size() > fcn )
			prop = segments[ fcn ].getMSProperty();
	}
    prop.setInstSamplingInterval( info.sampInterval );
    prop.setNumAverage( info.nAverage ); // workaround
    prop.setInstSamplingStartDelay( info.nSamplingDelay );
	prop.setSamplingInfo( info );

    ms.setMSProperty( prop );
    ms.resize( info.nSamples );
    ms.setIntensityArray( intensArray.data() + idx );

    if ( info.nSamples ) {
        double tic, base, rms;
        const double * intens = ms.getIntensityArray();
        tic = adportable::spectrum_processor::tic( info.nSamples, intens, base, rms );
        for ( size_t i = 0; i < info.nSamples; ++i )
            ms.setIntensity( i, intens[i] - base );
    }

    if ( ! massArray.empty() ) {
        ms.setMassArray( massArray.data() + idx );
    } else {
        // todo: add UI dialog box to ask those values
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
        
		// massArray.resize( size );
        std::pair< double, double > error;
        for ( size_t i = 0; i < ms.size(); ++i ) {
            double m_sqrt = adcontrols::MSCalibration::compute( calib.coeffs(), ms.getTime( i ) );
            double mz = m_sqrt * m_sqrt;
            ms.setMass( i, mz );
        }
        ms.setCalibration( coeffs );
    }
    return ms.size();
}

