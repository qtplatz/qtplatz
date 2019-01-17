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

#include "dialog.hpp"
#include "txtspectrum.hpp"
#include <adcontrols/datainterpreter.hpp>
#include <adcontrols/datainterpreterbroker.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/samplinginfo.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/string.hpp>
#include <adportable/textfile.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/interval.hpp>
#include <boost/tokenizer.hpp>
#include <algorithm>
#include <fstream>

using namespace adportable;
using namespace adcontrols;
using namespace adtextfile;

TXTSpectrum::TXTSpectrum()
{
}

bool
TXTSpectrum::load( const std::wstring& name, const Dialog& dlg )
{
	//bool hasMass( false );
	boost::filesystem::path path( name );

	boost::filesystem::ifstream in( path );
    if ( in.fail() )
        return false;

    typedef boost::char_separator<char> separator;
    typedef boost::tokenizer< separator > tokenizer;

    separator sep( ", \t", "", boost::drop_empty_tokens );

    std::array< std::vector<double>, 3 > cols; // (time, mass, intens) | ((time|mass), intensity)

    if ( dlg.hasDataInterpreter() ) {
        auto model = dlg.dataInterpreterClsid().toStdString();
        if ( auto interpreter = adcontrols::DataInterpreterBroker::make_datainterpreter( model ) ) {
            auto ms = std::make_shared< adcontrols::MassSpectrum >();
            if ( interpreter->compile_header( *ms, in ) ) {
                compiled_ = ms;
            }
        }
    }

    auto ignCols = dlg.ignoreColumns();
    auto skipLines = dlg.skipLines();

    do {
        std::string line;
        if ( textfile::getline( in, line ) ) {

            if ( skipLines ) {
                --skipLines;
                continue;
            }

            tokenizer tokens( line, sep );
            double values[3] = {0};
            int i(0);
            int col(0);
            for ( tokenizer::iterator it = tokens.begin(); it != tokens.end() && i < 3; ++it, ++col ) {
                const std::string& s = *it;
                if ( ! ignCols.contains ( col + 1 ) )
                    values[i++] = atof( s.c_str() ); // boost::lexical_cast<double> in gcc throw bad_cast for "9999" format.
            }
            if ( i == 2 ) {
#ifndef NDEBUG
                if ( cols[0].size() < 10 )
                    ADDEBUG() << line << "\t[" << cols[0].size() << "]\tvalues:" << values[0] << ", " << values[1];
#endif
                cols[0].push_back( values[0] ); // (time|mass)
                cols[1].push_back( values[1] * 1000 ); // intens
            } else if ( i == 3 ) {
                cols[0].push_back( values[0] ); // time
                cols[1].push_back( values[1] ); // mass
                cols[2].push_back( values[2] ); // intens
            }
        }

    } while( ! in.eof() );

    const size_t nCols = ( cols[2].size() == cols[0].size() ) ? 3 : 2;
    const size_t nSamples = cols[0].size();

    const std::vector<double>& iArray = ( nCols == 2 ) ? cols[1] : cols[2];

    if ( nSamples == 0 )
        return false;

    if ( compiled_ ) {
        const auto& tArray = cols[0];

        std::vector<SamplingInfo> segments;
        if ( analyze_segments( segments, tArray, compiled_.get() ) )
            validate_segments( segments, tArray );

        size_t idx = 0;
        size_t fcn = 0;
        for ( auto s: segments ) {
            std::shared_ptr< adcontrols::MassSpectrum > ptr( new adcontrols::MassSpectrum );
            if ( nCols == 2 ) { // no mass array
                ptr->setAcquisitionMassRange( 100, 1000 );
                std::vector<double> empty;
                idx += create_spectrum( *ptr, idx, s, tArray, empty, iArray, fcn++ );
            } else {
                ptr->setAcquisitionMassRange( cols[1].front(), cols[1].back() ); // mass
                idx += create_spectrum( *ptr, idx, s, tArray, cols[1], iArray, fcn++ );
            }
            spectra_.push_back( ptr );
        }
    } else {

        auto ptr = std::make_shared< adcontrols::MassSpectrum >();
        ptr->resize( nSamples );
        MSProperty prop;

        auto& iArray = ( nCols == 3 ) ? cols[2] : cols[1];

        if ( nCols == 3 || ( nCols == 2 && dlg.isTimeIntensity() ) ) {
            // convert 1st column to seconds
            auto& tArray = cols[0];
            adcontrols::metric::prefix source_prefix = dlg.dataPrefix();
            std::transform( tArray.begin(), tArray.end(), tArray.begin()
                            , [source_prefix] ( double t ) { return adcontrols::metric::scale_to_base<double>( t, source_prefix ); } );

            ptr->setTimeArray( tArray.data() );

            if ( nCols == 3 )
                ptr->setMassArray( cols[1].data() );

            double interval = ( tArray.back() - tArray.front() ) / ( tArray.size() - 1 );
            double delay = tArray[ 0 ];
            double zhalf = delay < 0 ? (-0.5) : 0.5;
            int32_t nDelay = uint32_t( tArray [ 0 ] / interval + zhalf );

            adcontrols::SamplingInfo info( interval, delay, nDelay, uint32_t( nSamples ), /* number of average */ 1, /*mode*/ 0 );
            info.setDelayTime( tArray[0] );
            prop.setSamplingInfo( info );

            if ( nCols == 2 ) {
                adportable::TimeSquaredScanLaw scanlaw( dlg.acceleratorVoltage(), 0.0, dlg.length() );
                for ( size_t i = 0; i < nSamples; ++i )
                    ptr->setMass( i, scanlaw.getMass( tArray[ i ], 0 ) );
            }

        } else {
            ptr->setMassArray( cols[0].data() );
        }

        if ( dlg.invertSignal() )
            std::transform( iArray.begin(), iArray.end(), iArray.begin(), []( double i ){ return -i; } );

        if ( dlg.correctBaseline() ) {
            double base, rms;
            adportable::spectrum_processor::tic( iArray.size(), iArray.data(), base, rms );
            std::transform( iArray.begin(), iArray.end(), iArray.begin(), [base]( double i ){ return i - base; } );
        }
        ptr->setIntensityArray( iArray.data() );
        ptr->setMSProperty( prop );
        ptr->setAcquisitionMassRange( ptr->getMass( 0 ), ptr->getMass( nSamples - 1 ) );

        spectra_.push_back( ptr );
    }
    return true;
}

bool
TXTSpectrum::analyze_segments( std::vector<adcontrols::SamplingInfo>& segments
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

    double startDelay = timeArray.front();
	// const unsigned long startDelay = static_cast<unsigned long>(  ( timeArray.front() * 1e12 /*s*/) / sampInterval + 0.5 );

    unsigned long nDelay = ( startDelay / sampInterval + ( startDelay < 0 ? (-0.5) : 0.5 ) );
    uint32_t nCount = 0;

    uint32_t idx = 0;
    for ( std::vector<double>::const_iterator it = timeArray.begin() + 1; it != timeArray.end(); ++it ) {
        ++nCount;
        ++idx;
        double x = it[ 0 ] - it[ -1 ];

        if ( ( x < 0 ) || ( x > double( sampInterval ) * 2.0e-12 ) ) {

			int mode = find_mode( segments.size() );
            segments.push_back( adcontrols::SamplingInfo( sampInterval * 1.0e-12, startDelay, nDelay, nCount, 1, mode ) );

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
    segments.push_back( adcontrols::SamplingInfo(sampInterval * 1.0e-12, startDelay, nDelay, nCount + 1, 1, mode ) );
    return true;
}

int
TXTSpectrum::find_mode( size_t idx ) const
{
	int mode = 0;
    if ( compiled_ ) {
		adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( *compiled_ );
		if ( idx < segs.size() )
			mode = segs[ idx ].getMSProperty().samplingInfo().mode();
    }
    return mode;
}

bool
TXTSpectrum::validate_segments( const std::vector<adcontrols::SamplingInfo>& segments, const std::vector<double>& timeArray )
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
                              , const adcontrols::SamplingInfo& info
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
    //prop.setInstSamplingInterval( info.sampInterval );
    //prop.setNumAverage( info.nAverage ); // workaround
    //prop.setInstSamplingStartDelay( info.nSamplingDelay );
	prop.setSamplingInfo( info );

    ms.setMSProperty( prop );
    ms.resize( info.nSamples() );
    ms.setIntensityArray( intensArray.data() + idx );

    if ( info.nSamples() ) {
        double tic, base, rms;
        const double * intens = ms.getIntensityArray();
        tic = adportable::spectrum_processor::tic( info.nSamples(), intens, base, rms );
        (void)tic;
        for ( size_t i = 0; i < info.nSamples(); ++i )
            ms.setIntensity( i, intens[i] - base );
    }


    if ( ! massArray.empty() ) {
        ms.setMassArray( massArray.data() + idx );
    } else {
        ms.setMassArray( timeArray.data() + idx );
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
        ms.setCalibration( coeffs );
    }
    ms.setAcquisitionMassRange( massArray.front(), massArray.back() );
    return ms.size();
}
