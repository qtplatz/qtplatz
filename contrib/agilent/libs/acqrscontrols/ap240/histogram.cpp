/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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
//#include "threshold_result.hpp"
#include "../threshold_result.hpp"
#include <adcontrols/timedigitalhistogram.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <algorithm>
#include <numeric>
#include <cassert>

using namespace acqrscontrols::ap240;

histogram::histogram() : serialnumber_( 0 )
                       , timeSinceEpoch_( 0 )
                       , trigger_count_( 0 )
                       , reset_requested_( true )
                       , wellKnownEvents_( 0 )
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

// deprecated
#if 0
size_t
histogram::append( const acqrscontrols::threshold_result_< ap240::waveform>& result )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    if ( reset_requested_ ||
         meta_.actualPoints != result.data()->meta_.actualPoints ||
         !adportable::compare<double>::approximatelyEqual( meta_.initialXOffset, result.data()->meta_.initialXOffset ) ||
         !adportable::compare<double>::approximatelyEqual( meta_.xIncrement, result.data()->meta_.xIncrement ) ) {
        
        meta_ = result.data()->meta_;
        method_ = result.data()->method_; // delay pulse + protocols
        
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

    if ( ! result.indices().empty() )
        std::for_each( result.indices().begin(), result.indices().end(), [&] ( uint32_t idx ) {  data_[ idx ] ++; });

    serialnumber_ = result.data()->serialnumber_;
    timeSinceEpoch_ = result.data()->timeSinceEpoch_;

    return ++trigger_count_;
}
#endif

size_t
histogram::append( const acqrscontrols::threshold_result_< acqrscontrols::ap240::waveform >& result )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    if ( reset_requested_ ||
         meta_.actualPoints != result.data()->meta_.actualPoints ||
         !adportable::compare<double>::approximatelyEqual( meta_.initialXOffset, result.data()->meta_.initialXOffset ) ||
         !adportable::compare<double>::approximatelyEqual( meta_.xIncrement, result.data()->meta_.xIncrement ) ) {
        
        meta_ = result.data()->meta_;
        method_ = result.data()->method_; // delay pulse + protocols
        
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

size_t
histogram::getHistogram( std::vector< std::pair<double, uint32_t> >& hist
                         , acqrscontrols::ap240::metadata& meta
                         , acqrscontrols::ap240::method& method
                         , std::pair<uint32_t, uint32_t>& serialnumber
                         , std::pair<uint64_t, uint64_t>& timeSinceEpoch )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    hist.clear();

    meta = meta_;
    serialnumber = std::make_pair( serialnumber_0_, serialnumber_ );
    timeSinceEpoch = std::make_pair( timeSinceEpoch_0_, timeSinceEpoch_ );

    double t0 = meta_.initialXOffset;
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
    auto it = hist.begin();
    
    while ( it != hist.end() ) {

        auto tail = it + 1;

        while ( tail != hist.end() && std::abs( tail->first - it->first ) < resolution )
            ++tail;

        std::pair< double, double > sum
            = std::accumulate( it, tail, std::make_pair( 0.0, 0.0 )
                               , []( const std::pair<double, double>& a, const std::pair<double, uint32_t>& b ) {
                                   return std::make_pair( a.first + (b.first * b.second), double(a.second + b.second) );
                               });
        
        times.push_back( sum.first / sum.second );
        intens.push_back( sum.second );

        it = tail;
    }
    
    return true;
}


//static
bool
histogram::average( const std::vector< std::pair< double, uint32_t > >& in, double resolution, std::vector < std::pair< double, uint32_t > >& out )
{
    auto it = in.begin();
    
    while ( it != in.end() ) {

        auto tail = it + 1;

        while ( tail != in.end() && std::abs( tail->first - it->first ) < resolution )
            ++tail;

        std::pair< double, double > sum
            = std::accumulate( it, tail, std::make_pair( 0.0, 0.0 )
                               , []( const std::pair<double, double>& a, const std::pair<double, uint32_t>& b ) {
                                   return std::make_pair( a.first + (b.first * b.second), double(a.second + b.second) );
                               });
        
        out.emplace_back( sum.first / sum.second, sum.second );

        it = tail;
    }
    
    return true;
}

void
histogram::move( adcontrols::TimeDigitalHistogram& x, bool reset )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    x.histogram().clear();
    
    x.setInitialXTimeSeconds( meta_.initialXTimeSeconds );
    x.setInitialXOffset( meta_.initialXOffset );

    ADDEBUG() << "meta_.initialXOffset: " << meta_.initialXOffset;
    
    x.setXIncrement( meta_.xIncrement );
    x.setActualPoints( meta_.actualPoints );
    x.setTrigger_count( trigger_count_ );
    x.setSerialnumber(  { serialnumber_0_, serialnumber_ } );
    x.setTimeSinceEpoch( { timeSinceEpoch_0_, timeSinceEpoch_ } );
    x.setWellKnownEvents( wellKnownEvents_ );
    x.setThis_protocol( method_.protocols() [ method_.protocolIndex() ] );
    x.setProtocolIndex( method_.protocolIndex(), uint32_t( method_.protocols().size() ) );

    double ext_trig_delay = x.this_protocol().delay_pulses().at( adcontrols::TofProtocol::EXT_ADC_TRIG ).first;
    
    for ( auto it = data_.begin(); it < data_.end(); ++it ) {
        if ( *it ) {
            double t = meta_.initialXOffset + std::distance( data_.begin(), it ) * meta_.xIncrement;
            x.histogram().emplace_back( t + ext_trig_delay, *it );
        }
    }
    reset_requested_ = reset;
}

