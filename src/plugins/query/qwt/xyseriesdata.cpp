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
#include <ratio>

XYSeriesData::XYSeriesData()
{
}

size_t
XYSeriesData::size() const
{
    return series_.size();
}

QPointF
XYSeriesData::sample( size_t idx ) const
{
    if ( idx < series_.size() ) 
        return series_[ idx ];

    return QPointF();
}

QRectF
XYSeriesData::boundingRect() const 
{
    return boundingRect_;
}

///////////////////////

XYScatterData::XYScatterData( QAbstractItemModel * model, int x, int y )
{
    int row = 0;
    double xMin, xMax, yMin, yMax;

    do {
        model->fetchMore( model->index( row, x ) );
        
        while ( model->index( row, x ).isValid() ) {
            QPointF p1( model->index( row, x ).data().toDouble(), model->index( row, y ).data().toDouble() );
            series_.emplace_back( p1 );
            if ( row == 0 ) {
                xMin = xMax = p1.x();
                yMin = yMax = p1.y();
            } else {
                xMin = std::min( xMin, p1.x() );
                xMax = std::max( xMax, p1.x() );
                yMin = std::min( yMin, p1.y() );
                yMax = std::max( yMax, p1.y() );                
            }
            ++row;
        }
        
    } while ( model->canFetchMore( model->index( row, x ) ) );

    boundingRect_.setCoords( xMin, yMin, xMax, yMax );
}

////////////////////
XYHistogramData::XYHistogramData( QAbstractItemModel * model, int x, int y )
{
    int row = 0;
    double xMin(0), xMax(0), yMax(0);    
    do {
        model->fetchMore( model->index( row, x ) );
        
        while ( model->index( row, x ).isValid() ) {
            QPointF p0( model->index( row, x ).data().toDouble(), 0.0 );
            QPointF p1( model->index( row, x ).data().toDouble(), model->index( row, y ).data().toDouble() );
            series_.emplace_back( p0 );
            series_.emplace_back( p1 );
            series_.emplace_back( p0 );
            if ( row == 0 ) {
                xMin = xMax = p1.x();
            } else {
                xMin = std::min( xMin, p1.x() );
                xMax = std::max( xMax, p1.x() );
                yMax = std::max( yMax, p1.y() );
            }

            ++row;
        }
        
    } while ( model->canFetchMore( model->index( row, x ) ) );

    boundingRect_.setCoords( xMin, 0.0, xMax, yMax );    
}

