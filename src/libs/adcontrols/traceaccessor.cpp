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

#include "traceaccessor.hpp"
#include "trace.hpp"
#include "chromatogram.hpp"
#include <adportable/debug.hpp>

using namespace adcontrols;

TraceAccessor::~TraceAccessor()
{
}

TraceAccessor::TraceAccessor() : maxfcn_( 0 )
{
}

TraceAccessor::TraceAccessor( const TraceAccessor& t ) : trace_( t.trace_ )
{
}

TraceAccessor&
TraceAccessor::operator += ( const TraceAccessor& t )
{
    if ( trace_.empty() )
        trace_ = t.trace_;
    else
        trace_.insert( trace_.end(), t.trace_.begin(), t.trace_.end() );
	if ( maxfcn_ < t.maxfcn_ )
		maxfcn_ = t.maxfcn_;
    return *this;
}

void
TraceAccessor::clear()
{
    trace_.clear();
}

void
TraceAccessor::push_back( int fcn, uint32_t pos, const seconds_t& t, double value, unsigned long events )
{
    if ( fcn > maxfcn_ )
        maxfcn_ = fcn;
    trace_.emplace_back( fcn, pos, events, t, value );
}

size_t
TraceAccessor::operator >> ( Trace& t ) const
{
    size_t n = 0;
    for ( const auto& d: trace_ ) {
        if ( d.fcn == t.fcn() ) {
            t.push_back( d.npos, d.x.seconds, d.y );
            ++n;
        }
    }
    return n;
}

void
TraceAccessor::copy_to( Trace& trace, int fcn )
{
    for ( const auto& d: trace_ ) {
        if ( d.fcn == fcn )
            trace.push_back( d.npos, d.x.seconds, d.y );
    }
}

void
TraceAccessor::copy_to( Chromatogram& c, int fcn )
{
    std::vector< double > x;
    std::vector< double > y;
    
    for ( const auto& d: trace_ ) {
        if ( d.fcn == fcn ) {
            x.push_back( d.x.seconds - trace_[0].x.seconds );
            y.push_back( d.y );
        }
    }
    c.resize( x.size() );
    c.setIntensityArray( y.data() );
    c.setTimeArray( x.data() );
}

