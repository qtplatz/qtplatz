/**************************************************************************
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include "xyseriesdata.hpp"
#include "waveform.hpp"
#include <AcqirisDataTypes.h>
#include <ratio>

#define VERTICAL_IN_VOLTS

XYSeriesData::XYSeriesData( std::shared_ptr< const waveform > d ) : d_( d )
{
    size_t size = d_->dataDesc_.returnedSamplesPerSeg;
    size_t ini = d_->dataDesc_.indexFirstPoint;
        
    double xMin = ( d_->delayTime_ ) * std::micro::den;
    double xMax = ( d_->delayTime_ + size * d_->dataDesc_.sampTime ) * std::micro::den;

    auto pair = std::minmax_element( d_->data_.begin() + ini, d_->data_.begin() + ini + size );

#if defined VERTICAL_IN_VOLTS
    boundingRect_.setCoords( xMin, d->toVolts( *pair.first ), xMax, d->toVolts( *pair.second ) );
#else
    boundingRect_.setCoords( xMin, *pair.first, xMax, *pair.second );
#endif
}

size_t
XYSeriesData::size() const
{
    return d_ ? d_->dataDesc_.returnedSamplesPerSeg : 0;
}

QPointF
XYSeriesData::sample( size_t idx ) const
{
    if ( d_ && ( idx < d_->dataDesc_.returnedSamplesPerSeg ) ) {

        idx += d_->dataDesc_.indexFirstPoint;

#if defined VERTICAL_IN_VOLTS
        return QPointF( ( d_->delayTime_ + idx * d_->dataDesc_.sampTime ) * std::micro::den, d_->toVolts( d_->data_[ idx ] ) );
#else
        return QPointF( ( d_->delayTime_ + idx * d_->dataDesc_.sampTime ) * std::micro::den, d_->data_[ idx ] );
#endif
    }
    return QPointF();
}

QRectF
XYSeriesData::boundingRect() const 
{
    return boundingRect_;
}

