// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "trace.hpp"
#include "traceaccessor.hpp"
#include <iostream>

using namespace adcontrols;

Trace::~Trace()
{
}

Trace::Trace() : pos_( -1 ), minY_(-10), maxY_(90)
{
}

Trace::Trace( const Trace& t ) : pos_( t.pos_ )
                               , minY_( t.minY_ )
                               , maxY_( t.maxY_ )
                               , traceX_( t.traceX_ )
                               , traceY_( t.traceY_ )
                               , events_( t.events_ )
{
}

void 
Trace::operator += ( const TraceAccessor& ta )
{
    if ( pos_ == size_t(-1) ) {
        pos_ = ta.pos();
    } else {
        // check if data array is continued
        size_t tailpos = pos_ + size();
        while ( tailpos < unsigned( ta.pos() ) ) { 
            traceY_.push_back( 0 );
            traceX_.push_back( traceX_.back() );
            events_.push_back( 0 );
        }
    }
    const double * pY = ta.getIntensityArray();
    const double * pX = ta.getTimeArray();
    const unsigned long * pE = ta.getEventsArray();
    size_t size = ta.size();
    for ( size_t i = 0; i < size; ++i ) {

        if ( pY[i] > maxY_ )
            maxY_ = pY[i];

        if ( pY[i] < minY_ )
            minY_ = pY[i];

        traceY_.push_back( pY[i] );
        events_.push_back( pE[i] );
        if ( pX )
            traceX_.push_back( pX[i] );
        else
            traceX_.push_back( timeutil::toMinutes( ta.getMinimumTime() + ta.sampInterval() * i ) );
    }
}

void
Trace::clear()
{
    traceX_.clear();
    traceY_.clear();
    events_.clear();
}

size_t
Trace::size() const
{
    return traceY_.size();
}

void
Trace::resize( size_t size )
{
    traceX_.resize( size );
    traceY_.resize( size );
    events_.resize( size );
}

const double *
Trace::getIntensityArray() const
{
    if ( traceY_.empty() )
        return 0;
    return &traceY_[0];
}

const double *
Trace::getTimeArray() const   // array of miniutes
{
    if ( traceX_.empty() )
        return 0;
    return &traceX_[0];
}

const unsigned long *
Trace::getEventsArray() const
{
    if ( events_.empty() )
        return 0;
    return &events_[0];
}

std::pair<double, double>
Trace::range_y() const
{
    return std::make_pair<double, double>( minY_, maxY_ );
}
