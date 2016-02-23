// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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
#include <numeric>
#include <tuple>

using namespace adcontrols;

Trace::~Trace()
{
}

Trace::Trace( int fcn, unsigned lower, unsigned upper ) : fcn_( fcn )
                                                        , lower_limit( lower )
														, upper_limit( upper )
                                                        , minY_( std::numeric_limits<double>::max() ), maxY_( std::numeric_limits<double>::lowest() )
                                                        , isCountingTrace_( false )
{
}

Trace::Trace( const Trace& t ) : fcn_( t.fcn_ )
                               , lower_limit( t.lower_limit )
							   , upper_limit( t.upper_limit )
                               , minY_( t.minY_ )
                               , maxY_( t.maxY_ )
                               , values_( t.values_ )
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
    values_ = t.values_;
    isCountingTrace_ = t.isCountingTrace_;
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
    if ( !values_.empty() && ( npos < std::get< data_number >( values_.back() ) ))
        return false;

    values_.emplace_back( npos, x, y, 0 );

    maxY_ = std::max( maxY_, y );
    minY_ = isCountingTrace_ ? 0 : std::min( minY_, y );

    if ( values_.size() > upper_limit ) {

        values_.erase( values_.begin(), values_.begin() + ( upper_limit - lower_limit ) );

        auto minmax = std::minmax_element( values_.begin(), values_.end(), [] ( const value_type& a, const value_type& b ) { return std::get<x_value>( a ) < std::get<y_value>( b ); } );;
        maxY_ = std::get<y_value>(*minmax.second);
        minY_ = isCountingTrace_ ? 0 : std::get<y_value>(*minmax.first);

    }
    return true;
}

bool
Trace::erase_before( size_t npos )
{
    auto it = std::upper_bound( values_.begin(), values_.end(), npos, [] ( size_t npos, const value_type& b ) { return npos < std::get<data_number>( b ); } );

    if ( it != values_.end() ) {

        values_.erase( values_.begin(), it );

        auto minmax = std::minmax_element( values_.begin(), values_.end(), [] ( const value_type& a, const value_type& b ) { return std::get<x_value>( a ) < std::get<y_value>( b ); } );;
        maxY_ = std::get<y_value>(*minmax.second);
        minY_ = isCountingTrace_ ? 0 : std::get<y_value>(*minmax.first);

        return true;
    }
    return false;
}

void
Trace::clear()
{
    values_.clear();
    minY_ = isCountingTrace_ ? 0 : std::numeric_limits<double>::max();
    maxY_ = std::numeric_limits<double>::lowest();
}

size_t
Trace::size() const
{
    return values_.size();
}

void
Trace::resize( size_t size )
{
    values_.resize( size );
}

double
Trace::x( size_t idx ) const
{
    return std::get< x_value >( values_.at( idx ) );
}

double
Trace::y( size_t idx ) const
{
    return std::get< y_value >( values_.at( idx ) );
}

std::pair< double, double >
Trace::xy( size_t idx ) const
{
    return std::make_pair( std::get< x_value >( values_.at( idx ) ), std::get< y_value >( values_.at( idx ) ) );
}

std::pair<double, double>
Trace::range_y() const
{
    return std::make_pair( minY_, maxY_ );
}

size_t
Trace::npos() const
{
    return values_.empty() ? 0 : std::get< data_number >( values_.back() );
}

void
Trace::setIsCountingTrace( bool f )
{
    if ( isCountingTrace_ != f ) {
        clear();
    }

    isCountingTrace_ = f;
    if ( isCountingTrace_ )
        minY_ = 0;
}

bool
Trace::isCountingTrace() const
{
    return isCountingTrace_;
}
