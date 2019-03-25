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

#include "trace.hpp"
#include <adacquire/constants.hpp>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <tuple>

using namespace adcontrols;

Trace::~Trace()
{
}

Trace::Trace( int fcn, unsigned lower, unsigned upper ) : upper_limit( upper )
														, lower_limit( lower )
                                                        , fcn_( fcn )
                                                        , minY_( std::numeric_limits<double>::max() )
                                                        , maxY_( std::numeric_limits<double>::lowest() )
                                                        , isCountingTrace_( false )
                                                        , enable_( true )
                                                        , injectTime_( 0 )
                                                        , yOffset_( 0 )
{
}

void
Trace::set_fcn( size_t n )
{
    fcn_ = static_cast<int>(n);
}

void
Trace::setProtocol( int proto )
{
    fcn_ = proto;
}

bool
Trace::append( size_t npos, double x, double y, uint32_t events )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    if ( !values_.empty() && ( npos < std::get< data_number >( values_.back() ) ))
        return false;

    values_.emplace_back( npos, x, y, events );

    maxY_ = std::max( maxY_, y );
    minY_ = isCountingTrace_ ? 0 : std::min( minY_, y );

    if ( values_.size() > upper_limit ) {

        values_.erase( values_.begin(), values_.begin() + ( upper_limit - lower_limit ) );

        auto minmax = std::minmax_element( values_.begin(), values_.end(), [] ( const value_type& a, const value_type& b ) {
                return std::get<y_value>( a ) < std::get<y_value>( b );
            } );
        maxY_ = std::get<y_value>(*minmax.second);
        minY_ = isCountingTrace_ ? 0 : std::get<y_value>(*minmax.first);

    }
    return true;
}

bool
Trace::erase_before( size_t npos )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    auto it = std::upper_bound( values_.begin(), values_.end(), npos, [] ( size_t npos, const value_type& b ) {
            return npos < std::get<data_number>( b ); } );

    if ( it != values_.end() ) {

        values_.erase( values_.begin(), it );

        auto minmax = std::minmax_element( values_.begin(), values_.end(), [] ( const value_type& a, const value_type& b ) {
                return std::get<x_value>( a ) < std::get<y_value>( b ); } );;
        maxY_ = std::get<y_value>(*minmax.second);
        minY_ = isCountingTrace_ ? 0 : std::get<y_value>(*minmax.first);

        return true;
    }
    return false;
}

void
Trace::clear()
{
    std::lock_guard< std::mutex > lock( mutex_ );

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
    std::lock_guard< std::mutex > lock( mutex_ );
    values_.resize( size );
}

double
Trace::x( size_t idx ) const
{
    if ( values_.size() > idx )
        return std::get< x_value >( values_.at( idx ) );
    return 0;
}

double
Trace::y( size_t idx ) const
{
    if ( values_.size() > idx )
        return std::get< y_value >( values_.at( idx ) );
    return 0;
}

std::pair< double, double >
Trace::xy( size_t idx ) const
{
    if ( values_.size() > idx )
        return std::make_pair( std::get< x_value >( values_.at( idx ) ), std::get< y_value >( values_.at( idx ) ) );
    return { 0, 0 };
}

uint32_t
Trace::events( size_t idx ) const
{
    if ( values_.size() > idx )
        return std::get< event_flags >( values_.at( idx ) );
    return 0;
}


std::pair<double, double>
Trace::range_y() const
{
    return std::make_pair( minY_ - yOffset_, maxY_ - yOffset_ );
}

size_t
Trace::npos() const
{
    return values_.empty() ? 0 : std::get< data_number >( values_.back() );
}

void
Trace::setIsCountingTrace( bool f )
{
    if ( isCountingTrace_ != f )
        clear();

    isCountingTrace_ = f;
    if ( isCountingTrace_ )
        minY_ = 0;
}

bool
Trace::isCountingTrace() const
{
    return isCountingTrace_;
}

void
Trace::setEnable( bool enable )
{
    enable_ = enable;
}

bool
Trace::enable() const
{
    return enable_;
}

void
Trace::setInjectTime( double t )
{
    injectTime_ = t;
}

double
Trace::injectTime() const
{
    return injectTime_;
}

const std::string&
Trace::legend() const
{
    return legend_;
}

void
Trace::setLegend( const std::string& legend )
{
    legend_ = legend;
}

void
Trace::setYOffset( double y )
{
    yOffset_ = y;
}

double
Trace::yOffset() const
{
    return yOffset_;
}
