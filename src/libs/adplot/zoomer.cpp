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

#include "zoomer.hpp"
#include <QMouseEvent>
#include <qwt_painter.h>
#include <qwt_picker_machine.h>
#include <QDebug>
#include <QList>
#include <QTimer>
#include <QWidget>
#include <boost/format.hpp>

using namespace adplot;

Zoomer::Zoomer( int xAxis, int yAxis, QWidget * canvas ) : QwtPlotZoomer( xAxis, yAxis, canvas )
                                                         , autoYScale_( false )
{
    // Shift+LeftButton: zoom out to full size
    setMousePattern( QwtEventPattern::MouseSelect2,  Qt::LeftButton, Qt::ShiftModifier );
    
    // Ctrl+LeftButton: zoom out by 1
    setMousePattern( QwtEventPattern::MouseSelect3, Qt::LeftButton, Qt::ControlModifier );
    // in addition to this, double click for zoom out by 1 via override widgetMouseDoubleClickEvent

    QPen pen( QColor( 0xff, 0, 0, 0x40 ) ); // transparent darkRed
    setRubberBandPen( pen );
    setRubberBand( CrossRubberBand );
    setTrackerMode( ActiveOnly );
    canvas->setMouseTracking( true );
    connect( this, &Zoomer::zoomed, this, &Zoomer::handleZoomed );
}

Zoomer::~Zoomer()
{
    disconnect( this, &Zoomer::zoomed, this, &Zoomer::handleZoomed );
}

void
Zoomer::autoYScale( bool f )
{
    autoYScale_ = f;
}

void
Zoomer::widgetMousePressEvent( QMouseEvent * e ) 
{
    p1_ = e->pos();
    QwtPlotZoomer::widgetMousePressEvent( e );
}

void
Zoomer::widgetMouseMoveEvent( QMouseEvent * e )
{
    if ( !isActive() ) {
        begin();
        append( e->pos() );
    } else
        move( e->pos() );
    QwtPlotPicker::widgetMouseMoveEvent( e );
}

void
Zoomer::widgetLeaveEvent( QEvent * )
{
    end();
}

// virtual
void
Zoomer::widgetMouseDoubleClickEvent( QMouseEvent * event )
{
	if ( mouseMatch( MouseSelect1, event ) )
		QwtPlotZoomer::zoom( -1 );

    QwtPlotPicker::widgetMouseDoubleClickEvent( event );
}

//virtual
bool
Zoomer::accept( QPolygon &pa ) const
{
    if ( pa.count() < 2 )
        return false;
  
    QRect rect = QRect( pa[0], pa[int( pa.count() ) - 1] );
    rect = rect.normalized();
  
    if ( rect.width() < 2 && rect.height() < 2 ) {
        return false;
    }

    pa.resize( 2 );

    const QRectF& rc = pickArea().boundingRect(); // view rect
    if ( rect.width() < minX ) {
        // make a virtical line
        pa[ 0 ] = QPoint( rc.left(), rect.top() );
        pa[ 1 ] = QPoint( rc.right(), rect.bottom() );
        return true;
    } else if ( rect.height() < minY ) {
        // make a horizontal line
        pa[ 0 ] = QPoint( rect.left(), rc.top() );
        pa[ 1 ] = QPoint( rect.right(), rc.bottom() );
    } else {
        pa[ 0 ] = rect.topLeft();
        pa[ 1 ] = rect.bottomRight();
    }
    return true;
}

QSizeF
Zoomer::minZoomSize() const
{
    QRectF rc = zoomBase();
    return QSizeF( rc.width() * 1.0e-6, rc.height() * 1.0e-6 );
}

void
Zoomer::drawRubberBand( QPainter *painter ) const
{
    if ( !isActive() || rubberBand() == NoRubberBand || rubberBandPen().style() == Qt::NoPen )
		return;

	const QPolygon pa = adjustedPoints( pickedPoints() );
    if ( pa.count() < 1 )
        return;

    const QRectF& rc = pickArea().boundingRect(); // view rect
    const QPoint p2 = pa[int( pa.count() - 1 )];

    QwtPainter::drawLine( painter, p2.x(), rc.top(), p2.x(), rc.bottom() );  // vertical @ 2nd point (automaticall drawn by picker)
    QwtPainter::drawLine( painter, rc.left(), p2.y(), rc.right(), p2.y() );  // horizontal @ 2nd point

	if ( pa.count() >= 2 ) {
        const QPoint p1 = pa[0];
        const QRect rect = QRect( p1, p2 ).normalized();
        
        if ( autoYScale_ || rect.height() < minY ) { // select horizontal (indicate by 2 vertical lines)
            QwtPainter::drawLine( painter, p1.x(), rc.top(), p1.x(), rc.bottom() );  // vertical @ 1st point
        } else if ( rect.width() < minX ) {
            QwtPainter::drawLine( painter, rc.left(), p1.y(), rc.right(), p1.y() );  // horizontal @ 1st point
        } else {
            QwtPainter::drawLine( painter, p1.x(), rc.top(), p1.x(), rc.bottom() );  // vertical @ 1st point
            QwtPainter::drawLine( painter, rc.left(), p1.y(), rc.right(), p1.y() );  // horizontal @ 1st point
        }
    }
}

QwtText
Zoomer::trackerTextF( const QPointF &pos ) const
{
    QColor bg( Qt::white );
    bg.setAlpha( 128 );
    QwtText text;
    if ( tracker2_ && ( p1_.x() || p1_.y() ) ) { // has 1st point
        text = tracker2_( invTransform( p1_ ), pos );
    } else if ( tracker1_ ) {
        text = tracker1_( pos );
    } else {
        text = QwtPlotZoomer::trackerTextF( pos );
    }
    text.setBackgroundBrush( QBrush( bg ) );
    return text;
}

void
Zoomer::zoom( const QRectF& rect )
{
    if ( autoYScale_ && autoYScaleHock_ ) {
        QRectF rc( rect );
        autoYScaleHock_( rc );
        QwtPlotZoomer::zoom( rc );
    } else {
        QwtPlotZoomer::zoom( rect );
    }
}

void
Zoomer::autoYScaleHock( std::function< void( QRectF& ) > f )
{
    autoYScaleHock_ = f;
}

void
Zoomer::tracker1( std::function<QwtText( const QPointF& )> f )
{
    tracker1_ = f;
}

void
Zoomer::tracker2( std::function<QwtText( const QPointF&, const QPointF& )> f )
{
    tracker2_ = f;
}

void
Zoomer::handleZoomed( const QRectF& )
{
    p1_ = QPoint( 0, 0 );  // invalidate p1_
}
