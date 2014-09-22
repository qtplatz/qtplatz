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

namespace adwplot {

    // spectral/chromatographical vector compressor

    template<class Polygon, class Point>
    class profile_compressor {
    public:
        Polygon compress1( const QwtScaleMap& xMap, const QwtScaleMap& yMap
                           , const QwtSeriesData< QPointF > * series, int from, int to, const size_t N ) const {
            
            std::vector< std::pair<int, double> > x;
            std::vector< std::pair<double, double> > y;

            std::vector<double> x_vec( to - from + 1 );
            std::vector<double> y_vec( to - from + 1 );
            for ( int i = from, k = 0; i < to; ++i, ++k ) {
                x_vec[ k ] = xMap.transform( series->sample( i ).x() );
                y_vec[ k ] = yMap.transform( series->sample( i ).y() );
            }

            Polygon polyline( (to - from + 1) * 2 );
            int pos = 0;

            auto it0 = x_vec.begin();
            while ( it0 != x_vec.end() ) {

                auto it1 = std::lower_bound( it0, x_vec.end(), *it0 + ( 1.0 / N ) );
                size_t idx0 = std::distance( x_vec.begin(), it0 );
                size_t idx1 = std::distance( x_vec.begin(), it1 );
                if ( it1 == x_vec.end() )
                    --idx1;
                size_t size = std::distance( it0, it1 );

                adportable::SGFilter filter( int( size ), adportable::SGFilter::Derivative1 );

                double d1 = filter( &y_vec[ idx0 + size / 2 ] );

                if ( d1 < 1.0 && d1 > -1.0) {
                    auto res = std::minmax_element( y_vec.begin() + idx0, y_vec.begin() + idx1 + 1 );
                    double dx = *it0 + ( *it1 - *it0 ) / 2.0;  // center value (average is not suitable for m/z)
                    polyline[ pos++ ] = Point( dx, *res.second ); // max (near bottom)
                    polyline[ pos++ ] = Point( dx, *res.first );
                } else {
                    for ( size_t i = idx0; i <= idx1; ++i )
                        polyline[ pos++ ] = Point( x_vec[ i ], y_vec[ i ] );
                }
                it0 = it1;
            }
            
            qDebug() << "drawLine data compress: " << pos << "/" << (to - from) + 1;

            polyline.resize( pos );
            return polyline;
        }

        Polygon compress0( const QwtScaleMap& xMap, const QwtScaleMap& yMap
                           , const QwtSeriesData< QPointF > * series, int from, int to, const size_t N ) const {
            
            std::vector< std::pair<int, double> > x;
            std::vector< std::pair<double, double> > y;

            int px = std::round( xMap.transform( series->sample( from ).x() * N ) ) - 1;
            
            for ( int i = from; i <= to; ++i ) {
                
                const Point& p = series->sample( i );
                
                double dx = xMap.transform( p.x() );
                double dy = yMap.transform( p.y() );
                
                int ix = (std::round( dx * N ));
                
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
                
                polyline[ pos++ ] = Point( dx, y[ i ].second ); // max (near bottom)
                polyline[ pos++ ] = Point( dx, y[ i ].first );  // min (near top)
                
            }

            qDebug() << "drawLine data compress: " << pos << "/" << (to - from) + 1;
            
            polyline.resize( pos );
            return polyline;
        }
    };


}

using namespace adwplot;

AdwPlotCurve::AdwPlotCurve( const QString& title ) : QwtPlotCurve( title )
                                                   , vectorCompression_( 0 )
{
    // curve_->setRenderHint( QwtPlotItem::RenderAntialiased );
    setPen( QPen( Qt::blue) );
    setStyle( QwtPlotCurve::Lines ); // continuum (or Stics)
    setLegendAttribute( QwtPlotCurve::LegendShowLine );
}

AdwPlotCurve::~AdwPlotCurve()
{
}

void
AdwPlotCurve::setVectorCompression( size_t n )
{
    vectorCompression_ = n;
}

void
AdwPlotCurve::drawLines( QPainter *painter,
                         const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                         const QRectF &canvasRect, int from, int to ) const
{
    if ( from > to )
        return;

    size_t vectorCompression = vectorCompression_;
    if ( QApplication::keyboardModifiers() & Qt::ControlModifier ) {
        if ( vectorCompression == 0 )
            vectorCompression = 3;
    }

    if ( auto device = painter->device() ) {
        qDebug() << "DpiX" << device->logicalDpiX() << ", " << device->physicalDpiX() << " width: " << device->width() << ", MM=" << device->widthMM();
    }
    int ix0 = from;
    int ix1 = to;
    while ( data()->sample( ix0 + 1 ).x() < xMap.s1() && ( ix0 + 1 ) < ix1 )
        ++ix0;
    while ( data()->sample( ix1 - 1 ).x() > xMap.s2() && ( ix1 - 1 ) > ix0 )
        --ix1;

    if ( vectorCompression == 0 || 
         ( (ix1 - ix0 + 1) < int( xMap.p2() - xMap.p1() * vectorCompression_ ) ) ) {
        
        QwtPlotCurve::drawLines( painter, xMap, yMap, canvasRect, from, to );

    } else {

        QRectF clipRect;
        
        if ( testPaintAttribute( ClipPolygons ) )  {

            qreal pw = qMax( qreal( 1.0 ), painter->pen().widthF() );
            clipRect = canvasRect.adjusted( -pw, -pw, pw, pw );

        }

        QPolygonF polyline = profile_compressor<QPolygonF, QPointF>().compress0( xMap, yMap, data(), ix0, ix1, vectorCompression );

        if ( testPaintAttribute( ClipPolygons ) )  {
            
            const QPolygonF clipped = QwtClipper::clipPolygonF( clipRect, polyline, false );
            QwtPainter::drawPolyline( painter, clipped );
            
        } else {
            
            QwtPainter::drawPolyline( painter, polyline );
            
        }
    }
}

