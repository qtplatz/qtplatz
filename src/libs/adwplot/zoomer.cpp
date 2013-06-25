// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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
#include <QDebug>

using namespace adwplot;

#if QWT_VERSION >= 0x060100
Zoomer::Zoomer( int xAxis, int yAxis, QWidget * canvas ) : QwtPlotZoomer( xAxis, yAxis, canvas )
#else
Zoomer::Zoomer( int xAxis, int yAxis, QwtPlotCanvas * canvas ) : QwtPlotZoomer( xAxis, yAxis, canvas )
#endif
                                                         , autoYScale_( false )
                                                         , rubberBand_( RectRubberBand ) 
{
    setTrackerMode(QwtPicker::AlwaysOff);
    // setRubberBand(QwtPicker::NoRubberBand);

    // LeftButton: zoom out by 1
    // unzoom 1 step is implemented as double-click 
    setMousePattern( QwtEventPattern::MouseSelect2,  Qt::LeftButton, Qt::ShiftModifier );
    
    // Ctrl+RightButton: zoom out to full size
    setMousePattern( QwtEventPattern::MouseSelect3,  Qt::LeftButton, Qt::ControlModifier );

    setRubberBand( QwtPicker::RectRubberBand );
    setRubberBandPen( QColor(Qt::green) );
    // setTrackerMode( QwtPicker::ActiveOnly );
    // setTrackerPen( QColor( Qt::white ) );
}

void
Zoomer::autoYScale( bool f )
{
    autoYScale_ = f;
}

void
Zoomer::zoom( const QRectF& rect )
{
    QRectF rc( rect );
    emit zoom_override( rc );
	QwtPlotZoomer::zoom( rc );
}

// virtual
void
Zoomer::widgetMouseDoubleClickEvent( QMouseEvent * event )
{
	if ( mouseMatch( MouseSelect1, event ) )
		QwtPlotZoomer::zoom( -1 );
	else
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
  
  const int minSize = 2;
  if ( rect.width() < minSize && rect.height() < minSize )
    return false;
  
  pa.resize( 2 );
  
  if ( rubberBand_ == HLineRubberBand ) {
#if QWT_VERSION <  0x060100
    const QRect& pRect = pickRect();
#else
    const QRectF& pRect = pickArea().boundingRect();
#endif
    pa[ 0 ] = QPoint( rect.left(), pRect.top() );
    pa[ 1 ] = QPoint( rect.right(), pRect.bottom() );
  } else if ( rubberBand_ == VLineRubberBand ) {
#if QWT_VERSION <  0x060100
    const QRect& pRect = pickRect();
#else
    const QRectF& pRect = pickArea().boundingRect();
#endif
    pa[ 0 ] = QPoint( pRect.left(), rect.top() );
    pa[ 1 ] = QPoint( pRect.right(), rect.bottom() );
  } else {
    pa[ 0 ] = rect.topLeft();
    pa[ 1 ] = rect.bottomRight();
  }
  
  return true;
}

void
Zoomer::drawRubberBand( QPainter *painter ) const
{
    if ( !isActive() || rubberBand() == NoRubberBand || rubberBandPen().style() == Qt::NoPen )
		return;

	const QPolygon pa = adjustedPoints( pickedPoints() );
	if ( pa.count() < 2 )
		return;

	const QPoint p1 = pa[0];
	const QPoint p2 = pa[int( pa.count() - 1 )];

	const QRect rect = QRect( p1, p2 ).normalized();
	if ( autoYScale_ || rect.height() < 20 ) {
		QwtPainter::drawLine( painter, p1.x(), p1.y(), p2.x(), p1.y() );  // horizontal
		const_cast<Zoomer *>(this)->rubberBand_ = HLineRubberBand;
	} else if ( rect.width() < 20 ) {
		QwtPainter::drawLine( painter, p1.x(), p1.y(), p1.x(), p2.y() );  // virtical
		const_cast<Zoomer *>(this)->rubberBand_ = VLineRubberBand;
	} else {
		QwtPainter::drawRect( painter, rect );
		const_cast<Zoomer *>(this)->rubberBand_ = RectRubberBand;
	}
}

