/**************************************************************************
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "histogram.hpp"
#include "threshold_result.hpp"
#include <adcontrols/timedigitalhistogram.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <boost/exception/all.hpp>
#include <algorithm>
#include <numeric>
#include <cassert>

using namespace acqrscontrols;
using namespace acqrscontrols::u5303a;

histogram::histogram() : serialnumber_( 0 )
                       , timeSinceEpoch_( 0 )
                       , wellKnownEvents_(0)
                       , trigger_count_( 0 )
                       , reset_requested_( true )
{
}

void
histogram::clear()
{
    reset_requested_ = true;
}

void
histogram::reset()
{
    reset_requested_ = true;
}

size_t
histogram::append( const threshold_result& result )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    if ( reset_requested_ ||
         meta_.actualPoints != result.data()->meta_.actualPoints ||
         ( std::abs( meta_.initialXOffset - result.data()->meta_.initialXOffset ) >= meta_.xIncrement * 2 ) ||
         !adportable::compare<double>::approximatelyEqual( meta_.xIncrement, result.data()->meta_.xIncrement ) ) {

        method_ = result.data()->method_; // delay pulse + protocols

        // ADDEBUG() << "append: " << method_.protocolIndex() << "/" << method_.protocols().size();

        meta_ = result.data()->meta_;

        assert ( meta_.actualPoints );

        data_.resize( meta_.actualPoints );

        reset_requested_ = false;
        trigger_count_ = 0;

        std::fill( data_.begin(), data_.end(), 0 );

        serialnumber_0_ = result.data()->serialnumber_;
        timeSinceEpoch_0_ = result.data()->timeSinceEpoch_;
        wellKnownEvents_ = 0;
    }

    assert( data_.size() );

    if ( method_.protocolIndex() != result.data()->method_.protocolIndex() )
        ADDEBUG() << "## ERROR protocol index missmatch: " << method_.protocolIndex() << " != " << result.data()->method_.protocolIndex();

    if ( ! result.indices().empty() )
        std::for_each( result.indices().begin(), result.indices().end(), [&] ( uint32_t idx ) {  data_[ idx ] ++; });

    serialnumber_ = result.data()->serialnumber_;
    timeSinceEpoch_ = result.data()->timeSinceEpoch_;
    wellKnownEvents_ |= result.data()->wellKnownEvents_;

    return ++trigger_count_;
}

size_t
histogram::trigger_count() const
{
    return trigger_count_;
}

double
histogram::triggers_per_sec() const
{
    return trigger_count_ / double( timeSinceEpoch_ - timeSinceEpoch_0_ ) * 1.0e-9;
}

void
histogram::move( adcontrols::TimeDigitalHistogram& x, bool reset )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    x.histogram().clear();

    x.setInitialXTimeSeconds( meta_.initialXTimeSeconds );
    x.setInitialXOffset( meta_.initialXOffset );
    x.setXIncrement( meta_.xIncrement );
    x.setActualPoints( meta_.actualPoints );
    x.setTrigger_count( trigger_count_ );
    x.setSerialnumber( std::make_pair( serialnumber_0_, serialnumber_ ) );
    x.setTimeSinceEpoch( std::make_pair( timeSinceEpoch_0_, timeSinceEpoch_ ) );
    x.setWellKnownEvents( wellKnownEvents_ );

    const size_t hard_index = method_.protocolIndex();
    const size_t safe_index = method_.protocolIndex() % method_.protocols().size();
    if ( hard_index != safe_index ) {
        ADDEBUG() << "Error: wrong protocol index of " << method_.protocolIndex() << " while num protocols are: " << method_.protocols().size();
        BOOST_THROW_EXCEPTION( std::range_error( "wrong protocol number" ) );
    }

    x.setThis_protocol( method_.protocols() [ safe_index ] );
    x.setProtocolIndex( safe_index, uint32_t( method_.protocols().size() ) );

    double ext_trig_delay = x.this_protocol().delay_pulses().at( adcontrols::TofProtocol::EXT_ADC_TRIG ).first;

    for ( auto it = data_.begin(); it < data_.end(); ++it ) {
        if ( *it ) {
            double t = meta_.initialXOffset + std::distance( data_.begin(), it ) * meta_.xIncrement;
            x.histogram().emplace_back( t + ext_trig_delay, *it );
        }
    }
    reset_requested_ = reset;
}

size_t
histogram::getHistogram( std::vector< std::pair<double, uint32_t> >& hist
                         , metadata& meta
                         , u5303a::method& method
                         , std::pair<uint32_t, uint32_t>& serialnumber
                         , std::pair<uint64_t, uint64_t>& timeSinceEpoch )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    hist.clear();

    meta = meta_;
    method = method_;
    serialnumber = std::make_pair( serialnumber_0_, serialnumber_ );
    timeSinceEpoch = std::make_pair( timeSinceEpoch_0_, timeSinceEpoch_ );

    // external trig. delay will to be taken account at TimeDigitalHistogram::translate()
    //double t0 = meta_.initialXOffset;
    for ( auto it = data_.begin(); it < data_.end(); ++it ) {
        if ( *it ) {
            double t = meta_.initialXOffset + std::distance( data_.begin(), it ) * meta_.xIncrement;
            hist.push_back( std::make_pair( t, *it ) );
        }
    }

    return trigger_count_;
}


//static
bool
histogram::average( const std::vector< std::pair< double, uint32_t > >& hist
                    , double resolution
                    , std::vector< double >& times
                    , std::vector< double >& intens )
{
    std::vector< std::pair< double, uint32_t > > time_merged;

    if ( adcontrols::TimeDigitalHistogram::average_time( hist, resolution, time_merged ) ) {

        times.resize( time_merged.size() );
        intens.resize( time_merged.size() );
        std::transform( time_merged.begin(), time_merged.end(), times.begin(), []( const std::pair< double, uint32_t >& a ){ return a.first; } );
        std::transform( time_merged.begin(), time_merged.end(), intens.begin(), []( const std::pair< double, uint32_t >& a ){ return a.second; } );
        return true;

    }

    return false;
}

//static
bool
histogram::average( const std::vector< std::pair< double, uint32_t > >& hist
                    , double resolution
                    , std::vector< std::pair< double, uint32_t > >& time_merged )
{
    return adcontrols::TimeDigitalHistogram::average_time( hist, resolution, time_merged );
}
