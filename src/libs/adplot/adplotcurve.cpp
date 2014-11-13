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

#include "adplotcurve.hpp"
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
#include <adportable/float.hpp>
#include <adportable/sgfilter.hpp>
#include <qalgorithms.h>
#include <qimage.h>
#include <qmath.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qpolygon.h>
#include <adportable/debug.hpp>
#include <boost/format.hpp>
#include <qdebug.h>
#include <QApplication>
#include <QtSvg/QSvgGenerator>

namespace adplot {

    // spectral/chromatographical vector compressor

    template<class Polygon, class Point>
    class profile_compressor {
        std::vector< std::pair<int, double> > x;
        std::vector< std::pair<double, double> > y;

    public:
        Polygon compress( const QwtScaleMap& xMap, const QwtScaleMap& yMap
                          , const QwtSeriesData< QPointF > * series, int from, int to, const size_t width ) {
            

            const double pw = (xMap.transform( series->sample( to ).x() ) - xMap.transform( series->sample( from ).x() )) / width;

            int px = std::round( xMap.transform( series->sample( from ).x() ) / pw ) - 1;
            
            for ( int i = from; i <= to; ++i ) {
                
                const Point& p = series->sample( i );
                
                double dx = xMap.transform( p.x() );
                double dy = yMap.transform( p.y() );
                
                int ix = (std::round( dx / pw ));
                
                if ( px != ix ) {
                    
                    x.push_back( std::make_pair( 1, dx ) );
                    y.push_back( std::make_pair( dy, dy ) );
                    px = ix;
                    
                } else {
                    
                    auto& xx = x.back();
                    xx.first++;
                    xx.second += dx;
                    
                    auto& yy = y.back();
                    yy.first = std::min( yy.first, dy );
                    yy.second = std::max( yy.second, dy );
                }
            }
            
            Polygon polyline( int( x.size() * 2 ) );
            int pos = 0;
            
            for ( size_t i = 0; i < x.size(); ++i ) {
                
                double dx = x[ i ].second / x[ i ].first;
                if ( i & 1 ) {
                    polyline[ pos++ ] = Point( dx, y[ i ].second ); // max (near bottom)
                    if ( !adportable::compare<double>::approximatelyEqual( y[ i ].second, y[ i ].first ) )
                        polyline[ pos++ ] = Point( dx, y[ i ].first );  // min (near top)
                }
                else {
                    polyline[ pos++ ] = Point( dx, y[ i ].first ); // max (near bottom)
                    if ( !adportable::compare<double>::approximatelyEqual( y[ i ].second, y[ i ].first ) )
                        polyline[ pos++ ] = Point( dx, y[ i ].second );  // min (near top)
                }
                
            }

            qDebug() << "drawLine data compress: " << pos << "/" << (to - from) + 1;
            
            polyline.resize( pos );
            return polyline;
        }

    };


}

using namespace adplot;

adPlotCurve::adPlotCurve( const QString& title ) : QwtPlotCurve( title )
                                                 , compress_( false )
{
    // curve_->setRenderHint( QwtPlotItem::RenderAntialiased );
    setPen( QPen( Qt::blue) );
    setStyle( QwtPlotCurve::Lines ); // continuum (or Stics)
    setLegendAttribute( QwtPlotCurve::LegendShowLine );
}

adPlotCurve::~adPlotCurve()
{
}

void
adPlotCurve::setVectorCompression( bool enable )
{
    compress_ = enable;
}

void
adPlotCurve::drawLines( QPainter *painter,
                         const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                         const QRectF &canvasRect, int from, int to ) const
{
    if ( from > to )
        return;

    int width = painter->device()->width();
    if ( auto device = painter->device() ) {
        if ( auto svgGenerator = dynamic_cast<QSvgGenerator *>(device) )
            width = svgGenerator->viewBox().width();
        else
            width = (device->widthMM() / 25.4) * device->physicalDpiX();
    }

    int ix0 = from;
    int ix1 = to;
    while ( data()->sample( ix0 + 1 ).x() < xMap.s1() && ( ix0 + 1 ) < ix1 )
        ++ix0;
    while ( data()->sample( ix1 - 1 ).x() > xMap.s2() && ( ix1 - 1 ) > ix0 )
        --ix1;

    if ( !compress_ || ( (ix1 - ix0 + 1) < width ) ) {
        
        QwtPlotCurve::drawLines( painter, xMap, yMap, canvasRect, from, to );

    } else {

        QRectF clipRect;
        
        if ( testPaintAttribute( ClipPolygons ) )  {

            qreal pw = qMax( qreal( 1.0 ), painter->pen().widthF() );
            clipRect = canvasRect.adjusted( -pw, -pw, pw, pw );

        }

        QPolygonF polyline = profile_compressor<QPolygonF, QPointF>().compress( xMap, yMap, data(), ix0, ix1, width );

        if ( testPaintAttribute( ClipPolygons ) )  {
            
            const QPolygonF clipped = QwtClipper::clipPolygonF( clipRect, polyline, false );
            QwtPainter::drawPolyline( painter, clipped );
            
        }
        else {
            
            QwtPainter::drawPolyline( painter, polyline );
            
        }

    }
}

