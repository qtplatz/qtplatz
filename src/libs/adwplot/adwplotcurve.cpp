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

#include "adwplotcurve.hpp"
#include "qwt_point_data.h"
#include "qwt_math.h"
#include "qwt_clipper.h"
#include "qwt_painter.h"
#include "qwt_scale_map.h"
#include "qwt_plot.h"
#include "qwt_curve_fitter.h"
#include "qwt_symbol.h"
#include "qwt_point_mapper.h"
#include <qpainter.h>
#include <qpixmap.h>
#include <qalgorithms.h>
#include <qmath.h>

using namespace adwplot;

AdwPlotCurve::AdwPlotCurve( const QString& title ) : QwtPlotCurve( title )
{
    // curve_->setRenderHint( QwtPlotItem::RenderAntialiased );
    setPen( QPen( Qt::blue) );
    setStyle( QwtPlotCurve::Lines ); // continuum (or Stics)
    setLegendAttribute( QwtPlotCurve::LegendShowLine );
    //attach( &plot );
}

AdwPlotCurve::~AdwPlotCurve()
{
}

void
AdwPlotCurve::drawLines( QPainter *painter,
                         const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                         const QRectF &canvasRect, int from, int to ) const
{
    if ( from > to )
        return;
    
    const bool doAlign = QwtPainter::roundingAlignment( painter );
    const bool doFit = false;  //( d_data->attributes & Fitted ) && d_data->curveFitter;
    const bool doFill = false; //( d_data->brush.style() != Qt::NoBrush )
                               // && ( d_data->brush.color().alpha() > 0 );

    QRectF clipRect;
    if ( testPaintAttribute( ClipPolygons ) )
    {
        qreal pw = qMax( qreal( 1.0 ), painter->pen().widthF() );
        clipRect = canvasRect.adjusted( -pw, -pw, pw, pw );
    }

    bool doIntegers = false;

#if QT_VERSION < 0x040800

    // For Qt <= 4.7 the raster paint engine is significantly faster
    // for rendering QPolygon than for QPolygonF. So let's
    // see if we can use it.

    if ( painter->paintEngine()->type() == QPaintEngine::Raster )
    {
        // In case of filling or fitting performance doesn't count
        // because both operations are much more expensive
        // then drawing the polyline itself

        if ( !doFit && !doFill )
            doIntegers = true; 
    }
#endif

    const bool noDuplicates = true; //d_data->paintAttributes & FilterPoints;

    QwtPointMapper mapper;
    mapper.setFlag( QwtPointMapper::RoundPoints, doAlign );
    mapper.setFlag( QwtPointMapper::WeedOutPoints, noDuplicates );
    mapper.setBoundingRect( canvasRect );
#if 0
    if ( doIntegers )
    {
        const QPolygon polyline = mapper.toPolygon( 
            xMap, yMap, data(), from, to );

        if ( d_data->paintAttributes & ClipPolygons )
        {
            const QPolygon clipped = QwtClipper::clipPolygon( 
                clipRect.toAlignedRect(), polyline, false );

            QwtPainter::drawPolyline( painter, clipped );
        }
        else
        {
            QwtPainter::drawPolyline( painter, polyline );
        }
    }
    else
#endif
    {
        QPolygonF polyline = mapper.toPolygonF( xMap, yMap, data(), from, to );
        // if ( doFit )
        //     polyline = d_data->curveFitter->fitCurve( polyline );

        if ( testPaintAttribute( ClipPolygons ) )
        {
            const QPolygonF clipped = QwtClipper::clipPolygonF( clipRect, polyline, false );

            QwtPainter::drawPolyline( painter, clipped );
        }
        else
        {
             QwtPainter::drawPolyline( painter, polyline );
        }

        if ( doFill )
        {
            fillCurve( painter, xMap, yMap, canvasRect, polyline );
        }
    }
}
