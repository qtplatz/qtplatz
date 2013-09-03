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

#include "traceaccessor.hpp"

using namespace adcontrols;

TraceAccessor::~TraceAccessor()
{
}

TraceAccessor::TraceAccessor()
{
}

TraceAccessor::TraceAccessor( const TraceAccessor& t ) : traces_( t.traces_ )
                                                       , events_( t.events_ )
{
}

void
TraceAccessor::clear()
{
    traces_.clear();
}

void
TraceAccessor::push_back( int fcn, uint32_t pos, const seconds_t& t, double value, unsigned long events )
{
    if ( size_t( fcn ) >= traces_.size() )
        traces_.resize( fcn + 1 );

    fcnTrace& trace = traces_[ fcn ];
	trace.pos_.push_back( pos );
    trace.traceX_.push_back( t.seconds );
    trace.traceY_.push_back( value );
    if ( events_.empty() || events_.back().second != events )
        events_.push_back( std::make_pair( t, events ) );
}

