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
#include <QDebug>
#include <limits>
#include <ratio>

using namespace adplot;

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

XYSeriesData::XYSeriesData( QAbstractItemModel * model, int x, int y )
{
    int row = 0;

    double xMin( std::numeric_limits< double >::max() ), xMax( std::numeric_limits< double >::lowest() )
        , yMin( std::numeric_limits< double >::max() ), yMax( std::numeric_limits< double >::lowest() );

    do {
        model->fetchMore( model->index( row, x ) );

        while ( model->index( row, x ).isValid() ) {

            QPointF p( model->index( row, x ).data().toDouble(), model->index( row, y ).data().toDouble() );

            series_.emplace_back( p );

            ++row;

            if ( xMin > p.x() )
                xMin = p.x();
            if ( xMax < p.x() )
                xMax = p.x();
            if ( yMin > p.y() )
                yMin = p.y();
            if ( yMax < p.y() )
                yMax = p.y();
        }

    } while ( model->canFetchMore( model->index( row, x ) ) );

    // RectF below looks like upside down, however qwtBoundingRect returns this way...

    boundingRect_ = QRectF( QPointF( xMin, yMin ), QPointF( xMax, yMax ) );
}

XYSeriesData&
XYSeriesData::operator << ( const QPointF& pt )
{
    series_.emplace_back( pt );
    if ( series_.size() == 1 ) {
        boundingRect_ = QRectF( pt, pt );
    } else {
        if ( boundingRect_.left() > pt.x() )
            boundingRect_.setLeft( pt.x() );
        if ( boundingRect_.right() < pt.x() )
            boundingRect_.setRight( pt.x() );

        if ( boundingRect_.bottom() > pt.y() )
            boundingRect_.setBottom( pt.y() );
        if ( boundingRect_.top() < pt.y() )
            boundingRect_.setTop( pt.y() );
    }
    return *this;
}

////////////////////
XYHistogramData::XYHistogramData( QAbstractItemModel * model, int x, int y ) : XYSeriesData( model, x, y )
{
}
