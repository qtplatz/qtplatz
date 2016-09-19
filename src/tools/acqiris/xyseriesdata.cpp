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

XYSeriesData::XYSeriesData( std::shared_ptr< const acqrscontrols::aqdrv4::waveform > d ) : d_( d )
{
    size_t size = d_->dataDesc().returnedSamplesPerSeg;
    size_t ini = d_->dataDesc().indexFirstPoint;
        
    double xMin = ( d_->delayTime() ) * std::micro::den;
    double xMax = ( d_->delayTime() + size * d_->dataDesc().sampTime ) * std::micro::den;

    if ( d->dataType() == sizeof( int8_t ) ) {
        auto pair = std::minmax_element( d_->begin<int8_t>() + ini, d_->end< int8_t >() );
        boundingRect_.setCoords( xMin, d->toVolts( *pair.first ), xMax, d->toVolts( *pair.second ) );
    } else if ( d->dataType() == sizeof( int16_t ) ) {
        auto pair = std::minmax_element( d_->begin<int16_t>() + ini, d_->end< int16_t >() );
        boundingRect_.setCoords( xMin, d->toVolts( *pair.first ), xMax, d->toVolts( *pair.second ) );        
    } else if ( d->dataType() == sizeof( int32_t ) ) {
        auto pair = std::minmax_element( d_->begin<int32_t>() + ini, d_->end< int32_t >() );
        boundingRect_.setCoords( xMin, d->toVolts( *pair.first ), xMax, d->toVolts( *pair.second ) );        
    }
}

size_t
XYSeriesData::size() const
{
    return d_ ? d_->dataDesc().returnedSamplesPerSeg : 0;
}

QPointF
XYSeriesData::sample( size_t idx ) const
{
    if ( d_ && ( idx < d_->size() ) ) {
            
#if defined VERTICAL_IN_VOLTS
        return QPointF( d_->time( idx ) * std::micro::den, d_->toVolts( (*d_)[ idx ] ) ); // mV
#else
        return QPointF( d_->time( idx ) * std::micro::den, *d_[ idx ] );        
#endif
    }
    return QPointF();
}

QRectF
XYSeriesData::boundingRect() const 
{
    return boundingRect_;
}

