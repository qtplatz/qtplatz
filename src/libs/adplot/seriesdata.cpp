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

#include "seriesdata.hpp"
#include <adcontrols/trace.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/massspectrum.hpp>

using namespace adplot;

SeriesData::SeriesData() : start_pos_(0)
{
    values_.reserve( 1024 * 8 ); // 8k
}

SeriesData::SeriesData( const SeriesData & t ) : start_pos_( t.start_pos_ )
                                               , values_( t.values_ ) 
{
}

size_t
SeriesData::size() const
{
    return values_.size();
}

QPointF
SeriesData::sample( size_t i ) const
{
    const size_t size = values_.size();
    size_t index = start_pos_ + i;
    if ( index >= size )
        index -= size;
    return values_[i];
}

QRectF
SeriesData::boundingRect() const
{
    QRectF rect;
    rect.setCoords( range_x_.first, range_y_.first, range_x_.second, range_y_.second );
    return rect;
}

void
SeriesData::setData( const adcontrols::Trace& trace )
{
    if ( trace.size() <= 2 )
        return;

    values_.resize( trace.size() );
    const double *x = trace.getTimeArray();
    const double *y = trace.getIntensityArray();

    for ( size_t i = 0; i < trace.size(); ++i )
        values_[i] = QPointF( x[i], y[i] );
}

void
SeriesData::setData( const adcontrols::Chromatogram& c )
{
    values_.resize( c.size() );

    range_x_ = adcontrols::Chromatogram::toMinutes( c.timeRange() );
    range_y_ = std::pair<double, double>( c.getMinIntensity(), c.getMaxIntensity() );

    const double *x = c.getTimeArray();
    const double *y = c.getIntensityArray();
    const size_t size = c.size();

    if ( x ) {
        for ( size_t i = 0; i < size; ++i )
            values_[i] = QPointF( adcontrols::Chromatogram::toMinutes( x[i] ), y[i] );
    } else {
        for ( size_t i = 0; i < size; ++i )
            values_[i] = QPointF( adcontrols::Chromatogram::toMinutes( c.timeFromDataIndex( i ) ), y[i] );
    }

}

void
SeriesData::setData( size_t size, const double * x, const double * y )
{
    values_.resize( size );
    for ( size_t i = 0; i < size; ++i )
        values_[i] = QPointF( x[i], y[i] );
}

