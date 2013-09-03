// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include <compiler/disable_dll_interface.h>
#include "trace.hpp"
#include "traceaccessor.hpp"
#include <iostream>
#include <algorithm>

using namespace adcontrols;

Trace::~Trace()
{
}

Trace::Trace() : fcn_( 0 ), minY_(-10), maxY_(90)
{
}

Trace::Trace( const Trace& t ) : fcn_( t.fcn_ )
                               , ulimits_( 4096 )
                               , minY_( t.minY_ )
                               , maxY_( t.maxY_ )
                               , traceX_( t.traceX_ )
                               , traceY_( t.traceY_ )
                               , events_( t.events_ )
{
}

void
Trace::nlimits( size_t n )
{
    ulimits_ = n;
}

size_t
Trace::nlimits() const
{
    return ulimits_;
}

void 
Trace::operator += ( const TraceAccessor& ta )
{
	if ( size_t( fcn_ ) >= ta.traces().size() )
		return;

	const TraceAccessor::fcnTrace& trace = ta.traces()[ fcn_ ];
	
	auto xIt = trace.traceX_.begin();
	for ( auto& y: trace.traceY_ ) {
		traceX_.push_back( *xIt++ ); // timeutil::toMinutes( *xIt ) ???
		traceY_.push_back( y );
		if ( maxY_ < y )
			maxY_ = y;
		if ( minY_ > y )
			minY_ = y;
	}

    if ( traceX_.size() > ulimits_ ) {
        const size_t n = ulimits_ / 4;
        traceX_.erase( traceX_.begin(), traceX_.begin() + n );
        traceY_.erase( traceY_.begin(), traceY_.begin() + n );
        events_.erase( events_.begin(), events_.begin() + n );
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
	double y0 = minY_;
	double y1 = maxY_;
    return std::pair<double, double>( y0, y1 );
}
