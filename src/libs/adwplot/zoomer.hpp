// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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

#ifndef ZOOMER_H
#define ZOOMER_H

#include <qwt_plot_zoomer.h>

class QwtPlotCanvas;

namespace adwplot {

    class Zoomer : public QwtPlotZoomer {
        Q_OBJECT
    public:
#if QWT_VERSION >= 0x060100
      Zoomer( int xAxis, int yAxis, QWidget * canvas );
#else // 0x060003 or earlier
      Zoomer( int xAxis, int yAxis, QwtPlotCanvas * canvas );
#endif
      void autoYScale( bool );

      // QwtPlotZoomer
      virtual void zoom( const QRectF& );
    private:
      bool autoYScale_;
      enum { HLineRubberBand, VLineRubberBand, RectRubberBand } rubberBand_;
    protected:
      // virtual void widgetMousePressEvent( QMouseEvent * );
      // virtual void widgetMouseReleaseEvent( QMouseEvent * );
      virtual void widgetMouseDoubleClickEvent( QMouseEvent * );
      // virtual void widgetMouseMoveEvent( QMouseEvent * );
      
      // QwtPlotZoomer
      virtual bool accept( QPolygon & ) const;
      
      // QwtPicker
      virtual void drawRubberBand( QPainter * ) const;
      
    signals:
        void zoom_override( QRectF& );
    };

}

#endif // ZOOMER_H
