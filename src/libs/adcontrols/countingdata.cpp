// This is a -*- C++ -*- header.
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

#include "countingdata.hpp"
#include <cmath>
#include <iostream>

using namespace adcontrols;

CountingPeak::CountingPeak() : apex(0,0)
                             , front(0,0)
                             , back( 0,0)
{
}

CountingPeak::CountingPeak( const CountingPeak& t ) : apex( t.apex )
                                                    , front( t.front )
                                                    , back( t.back )
{
}

double
CountingPeak::area() const
{
    return std::abs( height() ) * width();
}

double
CountingPeak::width() const
{
    return back.first - front.first;
}

double
CountingPeak::height() const
{
    return apex.second - ( front.second + back.second ) / 2.0;
}

CountingData::CountingData()
{
}

CountingData::CountingData( const CountingData& t ) : triggerNumber_( t.triggerNumber_ )
                                                    , protocolIndex_( t.protocolIndex_ )
                                                    , timeSinceEpoch_( t.timeSinceEpoch_ )
                                                    , elapsedTime_( t.elapsedTime_ )
                                                    , events_( t.events_ )
                                                    , threshold_( t.threshold_ )
                                                    , algo_( t.algo_ )
                                                    , peaks_( t.peaks_ )
{
}

CountingData&
CountingData::operator = ( const CountingData& t )
{
    triggerNumber_ = t.triggerNumber_;
    protocolIndex_ = t.protocolIndex_;
    timeSinceEpoch_ = t.timeSinceEpoch_;
    elapsedTime_ = t.elapsedTime_;
    events_ = t.events_;
    threshold_ = t.threshold_;
    algo_ = t.algo_;
    peaks_ = t.peaks_;
}


uint32_t
CountingData::triggerNumber() const
{
    return triggerNumber_;
}

uint32_t
CountingData::protocolIndex() const
{
    return protocolIndex_;
}

uint64_t
CountingData::timeSinceEpoch() const
{
    return timeSinceEpoch_;
}

double
CountingData::elapsedTime() const
{
    return elapsedTime_;
}

uint32_t
CountingData::events()
{
    return events_;
}

double
CountingData::threshold()
{
    return threshold_;
}

uint32_t
CountingData::algo() // 0:Absolute, 1:Average, 2:Deferential
{
    return algo_;
}

void
CountingData::setTriggerNumber( uint32_t v )
{
    triggerNumber_ = v;
}

void
CountingData::setProtocolIndex( uint32_t v )
{
    protocolIndex_ = v;
}

void
CountingData::setTimeSinceEpoch( uint64_t v )
{
    timeSinceEpoch_ = v;
}

void
CountingData::setElapsedTime( double v )
{
    elapsedTime_ = v;
}

void
CountingData::setEvents( uint32_t v )
{
    events_ = v;
}

void
CountingData::setThreshold( double v )
{
    threshold_ = v;
}

void
CountingData::setAlgo( uint32_t v ) // 0:Absolute, 1:Average, 2:Deferential
{
    algo_ = v;
}

