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
#include "qwt_clipper.h"
#include "qwt_curve_fitter.h"
#include "qwt_math.h"
#include "qwt_painter.h"
#include "qwt_pixel_matrix.h"
#include "qwt_plot.h"
#include "qwt_point_data.h"
#include "qwt_point_mapper.h"
#include "qwt_scale_map.h"
#include "qwt_symbol.h"
#include <qalgorithms.h>
#include <qimage.h>
#include <qmath.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qpolygon.h>
#include <adportable/debug.hpp>
#include <boost/format.hpp>

namespace adwplot {

    struct QwtRoundF {
        inline double operator()( double value ) {
            return static_cast<double>( qRound( value ) ); // int(value + 0.5) for positive
        }
    };

    template<class Polygon, class Point, class Round>
    class profile_compressor {
    public:
        Polygon compress( const QwtScaleMap& xMap, const QwtScaleMap& yMap
                          , const QwtSeriesData< QPointF > * series
                          , int from, int to, Round round ) {

            const QPointF sample0 = series->sample( from );
            const QPointF sample1 = series->sample( to );
            int x0 = round( xMap.transform( sample0.x() ) );
            int x1 = round( xMap.transform( sample1.x() ) );

            if ( (x1 - x0 + 1) > (to - from + 1) ) {

                Polygon polyline( to - from + 1 );
                Point *points = polyline.data();

                //const QPointF sample0 = series->sample( from );

                points[ 0 ].rx() = round( xMap.transform( sample0.x() ) );
                points[ 0 ].ry() = round( yMap.transform( sample0.y() ) );

                int pos = 0;
                for ( int i = from + 1; i <= to; i++ )  {
                    const QPointF sample = series->sample( i );

                    const Point p( round( xMap.transform( sample.x() ) ),
                                   round( yMap.transform( sample.y() ) ) );

                    if ( points[ pos ] != p )
                        points[ ++pos ] = p;
                }

                polyline.resize( pos + 1 );
                return polyline;
            }
            else {
                std::vector< std::pair<int, int> > a( x1 - x0 + 1 );
                int lastX = (-1);
                for ( int i = from; i <= to; ++i ) {

                    const QPointF sample = series->sample( i );

                    int x = round( xMap.transform( sample.x() ) );
                    int y = round( yMap.transform( sample.y() ) );
                    if ( lastX != x ) {
                        a[ x - x0 ].first = a[ x - x0 ].second = y;
                        lastX = x;
                    }
                    else {
                        a[ x - x0 ].first = std::min( a[ x - x0 ].first, y );
                        a[ x - x0 ].second = std::max( a[ x - x0 ].second, y );
                    }
                }
                
                Polygon polyline( int( a.size() * 2 ) );
                int x = x0;
                int pos = 0;
                for ( auto& y: a ) {
                    polyline[ pos++ ] = Point( x, y.second );
                    if ( y.first != y.second ) {
                        polyline[ pos++ ] = Point( x, y.first );
                    }
                    ++x;
                }
                if ( pos < polyline.size() )
                    polyline.resize( pos );

                return polyline;
            }
        }
    };


    template<class Polygon, class Point, class Round>
    static inline Polygon qwtToPolylineFiltered( const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                                                 const QwtSeriesData<QPointF> *series, 
                                                 int from, int to, Round round )
    {
        // in curves with many points consecutive points
        // are often mapped to the same position. As this might
        // result in empty lines ( or symbols hidden by others )
        // we try to filter them out

        Polygon polyline( to - from + 1 );
        Point *points = polyline.data();

        const QPointF sample0 = series->sample( from );

        points[0].rx() = round( xMap.transform( sample0.x() ) );
        points[0].ry() = round( yMap.transform( sample0.y() ) );

        int pos = 0;
        for ( int i = from + 1; i <= to; i++ )
        {
            const QPointF sample = series->sample( i );

            const Point p( round( xMap.transform( sample.x() ) ),
                           round( yMap.transform( sample.y() ) ) );

            if ( points[pos] != p )
                points[++pos] = p;
        }

        polyline.resize( pos + 1 );
        return polyline;
    };

    class PointMapper {
    public:
        QPolygonF toPolygonF( const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                              const QwtSeriesData<QPointF> *series, int from, int to ) const {
            return qwtToPolylineFiltered<QPolygonF, QPointF>( xMap, yMap, series, from, to, QwtRoundF() );
        }
    };

    
}

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
    
    // const bool doAlign = QwtPainter::roundingAlignment( painter );
    // const bool doFit = false;  //( d_data->attributes & Fitted ) && d_data->curveFitter;
    // const bool doFill = false; //( d_data->brush.style() != Qt::NoBrush )
                               // && ( d_data->brush.color().alpha() > 0 );

    QRectF clipRect;
    if ( testPaintAttribute( ClipPolygons ) )
    {
        qreal pw = qMax( qreal( 1.0 ), painter->pen().widthF() );
        clipRect = canvasRect.adjusted( -pw, -pw, pw, pw );
    }

    //const bool noDuplicates = testPaintAttribute( FilterPoints );

    //QwtPointMapper mapper;
    //PointMapper mapper;
    //mapper.setFlag( QwtPointMapper::RoundPoints, doAlign );
    //mapper.setFlag( QwtPointMapper::WeedOutPoints, noDuplicates );
    //mapper.setBoundingRect( canvasRect );

    //QPolygonF polyline = mapper.toPolygonF( xMap, yMap, data(), from, to );
    QPolygonF polyline = profile_compressor<QPolygonF, QPointF, QwtRoundF>().compress( xMap, yMap, data(), from, to, QwtRoundF() );

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
#if 0
    if ( doFill )
    {
        fillCurve( painter, xMap, yMap, canvasRect, polyline );
    }
#endif
}

