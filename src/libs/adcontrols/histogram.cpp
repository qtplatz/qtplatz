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

#include "histogram.hpp"
#include "massspectrum.hpp"
#include "massspectrometerbroker.hpp"
#include "msproperty.hpp"
#include "samplinginfo.hpp"
#include "scanlaw.hpp"
#if !defined NDEBUG
#include <adportable/debug.hpp>
#endif
#include <boost/format.hpp>
#include <cmath>

using namespace adcontrols;

std::shared_ptr< MassSpectrum >
histogram::make_profile( const MassSpectrum& ms, const MassSpectrometer& sp )
{
    auto profile = std::make_shared< MassSpectrum >( ms );

    for ( auto& tgt: segment_wrapper<>( *profile ) )
        histogram_to_profile( tgt, sp );

    return profile;
}

std::shared_ptr< MassSpectrum >
histogram::make_profile( const MassSpectrum& ms )
{
    auto profile = std::make_shared< MassSpectrum >( ms );

    for ( auto& tgt: segment_wrapper<>( *profile ) )
        histogram_to_profile( tgt );

    return profile;
}

void
histogram::histogram_to_profile( MassSpectrum& ms, const MassSpectrometer& spectrometer )
{
    auto& prop = ms.getMSProperty();
    auto& info = prop.samplingInfo();

    std::vector< double > counts, masses, times; // ( info.nSamples() ); // counts intensity array
    counts.reserve( info.nSamples() );
    masses.reserve( info.nSamples() );
    times.reserve( info.nSamples() );

    auto scanlaw = spectrometer.scanLaw();
    if ( !scanlaw )
        return;

    const double td = info.fSampInterval() * 2;
    double tp = info.delayTime();

    for ( size_t i = 0; i < ms.size(); ++i ) {
        
        double tc = ms.getTime( i );
        if ( ( tc - tp ) >= td ) {
            counts.emplace_back( 0 );
            times.emplace_back( tp + info.fSampInterval() ); // insert a sample next to previous data
            masses.emplace_back( scanlaw->getMass( tp + info.fSampInterval(), int( info.mode() ) ) );
            
            counts.emplace_back( 0 );
            times.emplace_back( tc - info.fSampInterval() ); // insert previous sample as 0 count
            masses.emplace_back( scanlaw->getMass( tc - info.fSampInterval(), int( info.mode() ) ) );
        }
        counts.emplace_back( ms.getIntensity( i ) );
        times.emplace_back( tc );
        masses.emplace_back( ms.getMass( i ) );
        tp = tc;
    }

    // terminate
    if ( ! counts.empty() ) {
        counts.emplace_back( 0 );
        times.emplace_back( times.back() + info.fSampInterval() );
        masses.emplace_back( scanlaw->getMass( times.back() + info.fSampInterval(), int( info.mode() ) ) );
    }
    
    ms.setCentroid( CentroidNone );
    ms.setMassArray( std::move( masses ) );
    ms.setTimeArray( std::move( times ) );
    ms.setIntensityArray( std::move( counts ) );
}

void
histogram::histogram_to_profile( MassSpectrum& ms )
{
    if ( ms.size() == 0 )
        return;
    
    auto& prop = ms.getMSProperty();
    auto& info = prop.samplingInfo();

    std::vector< double > counts, masses, times; // ( info.nSamples() ); // counts intensity array
    counts.reserve( info.nSamples() );
    masses.reserve( info.nSamples() );
    times.reserve( info.nSamples() );

    const double td = info.fSampInterval() * 2;
    double tp = info.delayTime();

    double deltaMass = 0.0001;
    
    for ( size_t i = 0; i < ms.size(); ++i ) {

        if ( i < ( ms.size() - 1 ) )
            deltaMass = ( ms.getMass( i + 1 ) - ms.getMass( i ) ) / ( ( ms.getTime( i + 1 ) - ms.getTime( i ) ) / info.fSampInterval() );

        double tc = ms.getTime( i );

        if ( ( tc - tp ) >= td ) {
            // end previous peak
            if ( ! counts.empty() ) {
                counts.emplace_back( 0 );
                times.emplace_back( tp + info.fSampInterval() );   // insert at the end of previous data
                masses.emplace_back( ms.getMass( i == 0 ? 0 : i - 1 ) + deltaMass );
            }

            // start this peak
            counts.emplace_back( 0 );
            times.emplace_back( tc - info.fSampInterval() );    // insert before next peak start
            masses.emplace_back( ms.getMass( i ) - deltaMass );
        }
        counts.emplace_back( ms.getIntensity( i ) );
        times.emplace_back( tc );
        masses.emplace_back( ms.getMass( i ) );
        tp = tc;
    }

    // terminate    
    if ( ! counts.empty() ) {
        counts.emplace_back( 0 );
        times.emplace_back( times.back() + info.fSampInterval() );
        masses.emplace_back( masses.back() + deltaMass );
    }

    ms.setCentroid( CentroidNone );
    ms.setMassArray( std::move( masses ) );
    ms.setTimeArray( std::move( times ) );
    ms.setIntensityArray( std::move( counts ) );
}



