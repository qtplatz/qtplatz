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

using namespace ads54j;

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
    assert( result.xmeta().actual_points_ );

    std::lock_guard< std::mutex > lock( mutex_ );

    reset_requested_ = meta_.trig_delay_counts_ != result.xmeta().trig_delay_counts_;

    if ( reset_requested_ ) {
        meta_ = result.xmeta();
        meta_.actual_averages_++;

        data_.resize( meta_.actual_points_ );

        reset_requested_ = false;

        std::fill( data_.begin(), data_.end(), 0 );

        serialnumber_0_ = result.data()->serialnumber();
        timeSinceEpoch_0_ = result.data()->epoch_time();
        wellKnownEvents_ = result.data()->well_known_events();
    }

    meta_.actual_averages_ += result.xmeta().actual_averages_ + 1;

    if ( result.size() )
        std::for_each( result.begin(), result.end(), [&] ( const adportable::counting::threshold_index& idx ) {  data_[ idx.apex ] ++; });

    serialnumber_ = result.data()->serialnumber();
    timeSinceEpoch_ = result.data()->epoch_time();
    wellKnownEvents_ |= result.data()->well_known_events();

    return meta_.actual_averages_;
}

size_t
histogram_adder::append( const waveform& pkd )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    if ( reset_requested_ ) {
        meta_ = pkd.xmeta();
        if ( meta_.actual_averages_ == 0 )
            meta_.actual_averages_++;

        data_.resize( meta_.actual_points_ );

        reset_requested_ = false;

        std::fill( data_.begin(), data_.end(), 0 );

        serialnumber_0_ = pkd.serialnumber();
        timeSinceEpoch_0_ = pkd.epoch_time();
        wellKnownEvents_ = pkd.well_known_events();
    }

    meta_.actual_averages_ += ( pkd.xmeta().actual_averages_ == 0 ) ? 1 : pkd.xmeta().actual_averages_;

    std::transform( pkd.begin(), pkd.end(), data_.begin(), data_.begin(), []( auto a, auto b ){ return a + b; } );

    serialnumber_ = pkd.serialnumber();
    timeSinceEpoch_ = pkd.epoch_time();
    wellKnownEvents_ |= pkd.well_known_events();

    return meta_.actual_averages_;
}


size_t
histogram_adder::actualAverage() const
{
    return meta_.actual_averages_;
}

double
histogram_adder::triggers_per_sec() const
{
    return meta_.actual_averages_ / double( timeSinceEpoch_ - timeSinceEpoch_0_ ) * 1.0e-9;
}

std::shared_ptr< adcontrols::TimeDigitalHistogram >
histogram_adder::fetch( bool reset )
{
    auto x = std::make_shared< adcontrols::TimeDigitalHistogram >();

    std::lock_guard< std::mutex > lock( mutex_ );

    x->histogram().clear();
    x->setInitialXTimeSeconds( double( meta_.clock_counts_ ) / double(meta_.clock_hz_ / 4) ); // seconds
    x->setInitialXOffset( meta_.time( 0 ) );
    x->setXIncrement( 1.0 / meta_.clock_hz_ );
    x->setActualPoints( data_.size() );
    x->setTrigger_count( meta_.actual_averages_ );
    x->setSerialnumber( std::make_pair( serialnumber_, serialnumber_0_ ) );
    x->setTimeSinceEpoch( std::make_pair( timeSinceEpoch_, timeSinceEpoch_0_ ) );
    x->setWellKnownEvents( wellKnownEvents_ );
    x->setThis_protocol( adcontrols::TofProtocol() ); //
    x->setProtocolIndex( 0, 1 );

    // double ext_trig_delay = 0; // x.this_protocol().delay_pulses().at( adcontrols::TofProtocol::EXT_ADC_TRIG ).first;

    for ( auto it = data_.begin(); it < data_.end(); ++it ) {
        if ( *it ) {
            double t = meta_.time( std::distance( data_.begin(), it ) );
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
    hist.setInitialXTimeSeconds( double( meta.clock_counts_ ) / double(meta.clock_hz_ / 4) ); // seconds
    // x.setInitialXOffset( meta_.initialXOffset );
    hist.setXIncrement( 1.0 / meta.clock_hz_ );
    hist.setActualPoints( pkd.size() );

    hist.setTrigger_count( meta.actual_averages_ );
    hist.setSerialnumber( std::make_pair( pkd.serialnumber(), pkd.serialnumber() ) );
    hist.setTimeSinceEpoch( std::make_pair( pkd.epoch_time(), pkd.epoch_time() ) );
    hist.setWellKnownEvents( pkd.well_known_events() );
    hist.setThis_protocol( adcontrols::TofProtocol() ); //
    hist.setProtocolIndex( 0, 1 );

    uint32_t index(0);
    for ( auto y: pkd ) {
        double t = ( index + meta.trig_delay_counts_ ) * (1.0/meta.clock_hz_);
        if ( y > 0 )
            hist.histogram().emplace_back( t, y );
        ++index;
    }
}
