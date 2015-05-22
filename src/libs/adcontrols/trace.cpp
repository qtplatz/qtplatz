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

#include <compiler/disable_dll_interface.h>
#include "trace.hpp"
#include <iostream>
#include <algorithm>

using namespace adcontrols;

Trace::~Trace()
{
}

Trace::Trace( int fcn, unsigned lower, unsigned upper ) : fcn_( fcn )
                                                        , lower_limit( lower )
														, upper_limit( upper )
                                                        , minY_( -10 ), maxY_( 90 )
{
}

Trace::Trace( const Trace& t ) : fcn_( t.fcn_ )
                               , lower_limit( t.lower_limit )
							   , upper_limit( t.upper_limit )
                               , minY_( t.minY_ )
                               , maxY_( t.maxY_ )
                               , traceX_( t.traceX_ )
                               , traceY_( t.traceY_ )
                               , events_( t.events_ )
{
}

Trace&
Trace::operator = ( const Trace& t )
{
    fcn_ = t.fcn_;
	lower_limit = t.lower_limit;
	upper_limit = t.upper_limit;
    minY_ = t.minY_;
    maxY_ = t.maxY_;
    traceX_ = t.traceX_;
    traceY_ = t.traceY_;
    events_ = t.events_;
    return *this;
}

void
Trace::set_fcn( size_t n )
{
    fcn_ = static_cast<int>(n);
}

bool
Trace::push_back( size_t npos, double x, double y )
{
    if ( npos_.empty() || ( npos_.back() < npos ) ) {

        npos_.push_back( npos );
        traceX_.push_back( x );
        traceY_.push_back( y );

        if ( maxY_ < y )
            maxY_ = y;
        if ( minY_ > y )
            minY_ = y;
        if ( npos_.size() > upper_limit ) {
            npos_.erase( npos_.begin(), npos_.begin() + ( upper_limit - lower_limit ) );
            traceX_.erase( traceX_.begin(), traceX_.begin() + ( upper_limit - lower_limit ) );
            traceY_.erase( traceY_.begin(), traceY_.begin() + ( upper_limit - lower_limit ) );
        }
        return true;
    }
    return false;
}

bool
Trace::erase_before( size_t npos )
{
    auto it = std::upper_bound( npos_.begin(), npos_.end(), npos );
    if ( it != npos_.end() ) {

        size_t n = std::distance( npos_.begin(), it );

        npos_.erase( npos_.begin(), npos_.begin() + n );
        traceX_.erase( traceX_.begin(), traceX_.begin() + n );
        traceY_.erase( traceY_.begin(), traceY_.begin() + n );
        
        auto y = std::minmax( traceY_.begin(), traceY_.end() );

        minY_ = *y.first;
        maxY_ = *y.second;
        
        return true;
    }
    return false;
}

void
Trace::clear()
{
    traceX_.clear();
    traceY_.clear();
    npos_.clear();
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
    npos_.resize( size );
}

const double *
Trace::getIntensityArray() const
{
    return traceY_.data();
}

const double *
Trace::getTimeArray() const   // array of miniutes
{
    return traceX_.data();
}

const unsigned long *
Trace::getEventsArray() const
{
    return 0;
}

std::pair<double, double>
Trace::range_y() const
{
	double y0 = minY_;
	double y1 = maxY_;
    return std::pair<double, double>( y0, y1 );
}

size_t
Trace::npos() const
{
    return npos_.empty() ? 0 : npos_.back();
}
