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
#include "threshold_result.hpp"
#include <adportable/float.hpp>
#include <algorithm>

using namespace ap240;

histogram::histogram() : trigger_count_(0)
                       , tp_( std::chrono::steady_clock::now() )
{
}

void
histogram::clear()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    meta_.actualPoints = 0;
    tp_ = std::chrono::steady_clock::now();
}

void
histogram::append( const threshold_result& result )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    
    if ( meta_.actualPoints != result.data->meta_.actualPoints ||
         !adportable::compare<double>::approximatelyEqual( meta_.initialXOffset, result.data->meta_.initialXOffset ) ||
         !adportable::compare<double>::approximatelyEqual( meta_.xIncrement, result.data->meta_.xIncrement ) ) {
        
        trigger_count_ = 0;
        meta_ = result.data->meta_;
        
        data_.resize( meta_.actualPoints );
        std::fill( data_.begin(), data_.end(), 0 );
    }
    std::for_each( result.index.begin(), result.index.end(), [this]( uint32_t idx ){
            data_[ idx ] ++; });
    ++trigger_count_;
}

size_t
histogram::trigger_count() const
{
    return trigger_count_;
}

double
histogram::triggers_per_sec() const
{
    return trigger_count_ / std::chrono::duration<double>(std::chrono::steady_clock::now() - tp_).count();
}

size_t
histogram::getHistogram( std::vector< std::pair<double, uint32_t> >& histogram, ap240::metadata& meta )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    meta = meta_;
    histogram.clear();
    double t0 = meta_.initialXOffset;
    for ( auto it = data_.begin(); it < data_.end(); ++it ) {
        if ( *it ) {
            double t = meta_.initialXOffset + std::distance( data_.begin(), it ) * meta_.xIncrement;
            histogram.push_back( std::make_pair( t, *it ) );
        }
    }

    return trigger_count_;
}


