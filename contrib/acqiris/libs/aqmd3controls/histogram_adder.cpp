/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "histogram_adder.hpp"
#include "pkd_result.hpp"
#include <adcontrols/timedigitalhistogram.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <algorithm>
#include <numeric>
#include <cassert>

using namespace aqmd3controls;

histogram_adder::histogram_adder() : serialnumber_( 0 )
                                   , timeSinceEpoch_( 0 )
                                   , wellKnownEvents_(0)
                                   , reset_requested_( true )
{
}

void
histogram_adder::clear()
{
    reset_requested_ = true;
}

void
histogram_adder::reset()
{
    reset_requested_ = true;
}

size_t
histogram_adder::append( const pkd_result& result )
{
    assert( result.xmeta().actualPoints );

    std::lock_guard< std::mutex > lock( mutex_ );

    reset_requested_ = meta_.actualPoints != result.xmeta().actualPoints ||
        int64_t( meta_.initialXOffset * 1.0e9 ) != int64_t( result.xmeta().initialXOffset * 1.0e9 );

    if ( reset_requested_ ) {
        meta_ = result.xmeta();
        meta_.actualAverages++;

        data_.resize( meta_.actualPoints );

        reset_requested_ = false;

        std::fill( data_.begin(), data_.end(), 0 );

        serialnumber_0_ = result.data()->serialnumber();
        timeSinceEpoch_0_ = result.data()->epoch_time();
        wellKnownEvents_ = result.data()->well_known_events();
    }

    meta_.actualAverages += result.xmeta().actualAverages + 1;

    if ( result.size() )
        std::for_each( result.begin(), result.end(), [&] ( const adportable::counting::threshold_index& idx ) {  data_[ idx.apex ] ++; });

    serialnumber_ = result.data()->serialnumber();
    timeSinceEpoch_ = result.data()->epoch_time();
    wellKnownEvents_ |= result.data()->well_known_events();

    return meta_.actualAverages;
}

size_t
histogram_adder::append( const waveform& pkd )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    if ( reset_requested_ ) {
        meta_ = pkd.xmeta();
        if ( meta_.actualAverages == 0 )
            meta_.actualAverages++;

        data_.resize( meta_.actualPoints );

        reset_requested_ = false;

        std::fill( data_.begin(), data_.end(), 0 );

        serialnumber_0_ = pkd.serialnumber();
        timeSinceEpoch_0_ = pkd.epoch_time();
        wellKnownEvents_ = pkd.well_known_events();
    }

    meta_.actualAverages += ( pkd.xmeta().actualAverages == 0 ) ? 1 : pkd.xmeta().actualAverages;

    std::transform( pkd.begin(), pkd.end(), data_.begin(), data_.begin(), []( auto a, auto b ){ return a + b; } );

    serialnumber_ = pkd.serialnumber();
    timeSinceEpoch_ = pkd.epoch_time();
    wellKnownEvents_ |= pkd.well_known_events();

    return meta_.actualAverages;
}


size_t
histogram_adder::actualAverage() const
{
    return meta_.actualAverages;
}

double
histogram_adder::triggers_per_sec() const
{
    return meta_.actualAverages / double( timeSinceEpoch_ - timeSinceEpoch_0_ ) * 1.0e-9;
}

std::shared_ptr< adcontrols::TimeDigitalHistogram >
histogram_adder::fetch( bool reset )
{
    auto x = std::make_shared< adcontrols::TimeDigitalHistogram >();

    std::lock_guard< std::mutex > lock( mutex_ );

    x->histogram().clear();
    x->setInitialXTimeSeconds( meta_.initialXTimeSeconds );
    x->setInitialXOffset( meta_.initialXOffset );
    x->setXIncrement( meta_.xIncrement );
    x->setActualPoints( meta_.actualPoints );
    x->setTrigger_count( meta_.actualAverages );
    x->setSerialnumber( std::make_pair( serialnumber_, serialnumber_0_ ) );
    x->setTimeSinceEpoch( std::make_pair( timeSinceEpoch_, timeSinceEpoch_0_ ) );
    x->setWellKnownEvents( wellKnownEvents_ );
    x->setThis_protocol( adcontrols::TofProtocol() ); //
    x->setProtocolIndex( 0, 1 );

    // double ext_trig_delay = 0; // x.this_protocol().delay_pulses().at( adcontrols::TofProtocol::EXT_ADC_TRIG ).first;

    for ( auto it = data_.begin(); it < data_.end(); ++it ) {
        if ( *it ) {
            double t = std::distance( data_.begin(), it ) * meta_.xIncrement + meta_.initialXOffset;
            x->histogram().emplace_back( t, *it );
        }
    }
    reset_requested_ = reset;

    return x;
}

//static
void
histogram_adder::translate( adcontrols::TimeDigitalHistogram& hist, const waveform& pkd )
{
    const auto& meta = pkd.xmeta();
    hist.histogram().clear();

    hist.setInitialXTimeSeconds( meta.initialXTimeSeconds );
    hist.setInitialXOffset( meta.initialXOffset );
    hist.setXIncrement( meta.xIncrement );
    hist.setActualPoints( pkd.size() );

    hist.setTrigger_count( meta.actualAverages );
    hist.setSerialnumber( std::make_pair( pkd.serialnumber(), pkd.serialnumber() ) );
    hist.setTimeSinceEpoch( std::make_pair( pkd.epoch_time(), pkd.epoch_time() ) );
    hist.setWellKnownEvents( pkd.well_known_events() );
    hist.setThis_protocol( adcontrols::TofProtocol() ); //
    hist.setProtocolIndex( 0, 1 );

    uint32_t index(0);
    for ( auto y: pkd ) {
        if ( y > 0 )
            hist.histogram().emplace_back( pkd.time( index ), y );
        ++index;
    }
}
