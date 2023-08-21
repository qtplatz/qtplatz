/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC
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
#include "segment_wrapper.hpp"
#include <adportable/debug.hpp>
#include <adportable/scanlaw_solver.hpp>
#include <boost/format.hpp>
#include <cmath>

namespace {

    template< typename T >
    struct mass_assigner {
        const T& t_;
        mass_assigner( const T& t ) : t_( t ) {}
        inline double operator()( double time, int mode, double t, double m ) const;
    };

    template<> double mass_assigner< adportable::scanlaw_solver >::operator()( double time, int, double t, double m ) const {
        auto delta = std::abs( t_.mass( time ) - t_.mass( t ) );
        // if ( time < 15e-6 )
        //     ADDEBUG() << "time: " << (time*1e6) << ", " << (t*1e6) << ", dt: " << ( time - t )*1e9 << "ns\tdelta m: " << delta * 1000;
        if ( time < t ) { // adding in front of the peak
            return m - delta;
        } else {          // adding to back of the peak
            return m + delta;
        }
    }
    ///////
}

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
    const auto& prop = ms.getMSProperty();
    const auto& info = prop.samplingInfo();
    std::vector< double > counts( info.nSamples() ), times( info.nSamples() ), masses( info.nSamples() );

    counts.assign( info.nSamples(), 0 );
    for ( size_t i = 0; i < info.nSamples(); ++i ) {
        double t = info.delayTime() + info.fSampInterval() * i;
        times.at(i) = t;
        masses.at(i) = spectrometer.assignMass( t, int( info.mode() ) );
    }

    for ( size_t j = 0; j < ms.size(); ++j ) {
        size_t i = (ms.time(j) - info.delayTime()) / info.fSampInterval() + 0.5;
        counts.at(i) = ms.intensity(j);
    }

    ms.setCentroid( CentroidNone );
    ms.setMassArray( std::move( masses ) );
    ms.setTimeArray( std::move( times ) );
    ms.setIntensityArray( std::move( counts ) );
}

void
histogram::histogram_to_profile( MassSpectrum& ms )
{
    if ( ms.size() < 2 )
        return;

    using adportable::scanlaw_solver;

    scanlaw_solver assign( {ms.mass(0), ms.mass(ms.size() - 1)}, {ms.time(0), ms.time(ms.size() - 1)} );

    const auto& prop = ms.getMSProperty();
    const auto& info = prop.samplingInfo();
    std::vector< double > counts( info.nSamples() ), times( info.nSamples() ), masses( info.nSamples() );

    counts.assign( info.nSamples(), 0 );
    masses.assign( info.nSamples(), 0 );
    for ( size_t i = 0; i < info.nSamples(); ++i )
        times.at(i)  = info.delayTime() + info.fSampInterval() * i;

    for ( size_t j = 0; j < ms.size(); ++j ) {
        size_t i = (ms.time(j) - info.delayTime()) / info.fSampInterval() + 0.5;
        counts.at(i) = ms.intensity(j);
        masses.at(i) = ms.mass(j);
    }

    auto it = masses.begin();
    while ( it = std::find( it, masses.end(), double(0) ), it != masses.end() ) {
        auto ite = std::find_if(it, masses.end(), [](const double& m){ return m > 0; });

        if ( it == masses.begin() || ite == masses.end() ) {
            while ( it != ite ) {
                *it = assign.mass( times.at( std::distance( masses.begin(), it ) ) );
                ++it;
            }
        } else {
            auto [m,n] = std::make_pair(std::distance( masses.begin(), it ), std::distance( masses.begin(), ite ));
            for ( size_t i = m; i < n; ++i )
                masses.at(i) = assign.mass( times.at(i) );
            it = ite;
        }
    }

    ms.setCentroid( CentroidNone );
    ms.setMassArray( std::move( masses ) );
    ms.setTimeArray( std::move( times ) );
    ms.setIntensityArray( std::move( counts ) );
}

void
histogram::histogram_to_profile( MassSpectrum& ms, std::function< double(double, int, double, double) > assigner )
{
    auto& prop = ms.getMSProperty();
    auto& info = prop.samplingInfo();

    std::vector< double > counts, masses, times; // ( info.nSamples() ); // counts intensity array
    counts.reserve( info.nSamples() );
    masses.reserve( info.nSamples() );
    times.reserve( info.nSamples() );

    const double td = info.fSampInterval();
    //double tp = info.delayTime();
    double tp = ms.time(0) - td;

    // initial 0
    counts.emplace_back( 0 );
    times.emplace_back( tp );
    masses.emplace_back( assigner( tp, int( info.mode() ), ms.time(0), ms.mass(0) ) );

    // first data
    counts.emplace_back( ms.intensity( 0 ) );
    times.emplace_back( ms.time( 0 ) );
    masses.emplace_back( ms.mass( 0 ) );

    for ( size_t i = 1; i < ms.size(); ++i ) {

        double tc = ms.time( i );
        if ( ( tc - tp ) >= td * 1.05 ) {
            // insert zero after the peak cluster
            counts.emplace_back( 0 );
            times.emplace_back( tp + info.fSampInterval() );
            masses.emplace_back( assigner( times.back(), int( info.mode() ), tp, masses.back() ) );

            // insert 1-sample point advance data with 0 count
            counts.emplace_back( 0 );
            times.emplace_back( tc - info.fSampInterval() );
            masses.emplace_back( assigner( times.back(), int( info.mode() ), tc, ms.mass( i ) ) );
        }
        counts.emplace_back( ms.intensity( i ) );
        times.emplace_back( tc );
        masses.emplace_back( ms.mass( i ) );
        tp = tc;
    }

    // terminate
    if ( ! counts.empty() ) {
        counts.emplace_back( 0 );
        times.emplace_back( times.back() + info.fSampInterval() );
        masses.emplace_back( assigner( times.back(), int( info.mode() ), tp, masses.back() ) );
    }

    ms.setCentroid( CentroidNone );
    ms.setMassArray( std::move( masses ) );
    ms.setTimeArray( std::move( times ) );
    ms.setIntensityArray( std::move( counts ) );
}
