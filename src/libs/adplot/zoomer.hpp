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

#ifndef ZOOMER_H
#define ZOOMER_H

#include <qwt_plot_zoomer.h>
#include <functional>

class QwtPlotCanvas;

namespace adplot {

    class Zoomer : public QwtPlotZoomer {
        Q_OBJECT
    public:
        ~Zoomer();
        Zoomer( int xAxis, int yAxis, QWidget * canvas );

        void autoYScale( bool );
        void autoYScaleHock( std::function< void( QRectF& ) > );
        void tracker1( std::function<QwtText( const QPointF& )> );
        void tracker2( std::function<QwtText( const QPointF&, const QPointF& )> );

        // QwtPlotZoomer
        void zoom( const QRectF& rect ) override;

    private:
        static const int minX = 20;
        static const int minY = 20;

        bool autoYScale_;
        QPoint p1_;
        std::function<void( QRectF& )> autoYScaleHock_;
        std::function<QwtText( const QPointF& )> tracker1_;
        std::function<QwtText( const QPointF&, const QPointF& )> tracker2_;
    protected:
        void widgetMousePressEvent( QMouseEvent* ) override;
        void widgetMouseDoubleClickEvent( QMouseEvent * ) override;
        void widgetMouseMoveEvent( QMouseEvent * ) override;
        void widgetLeaveEvent( QEvent * ) override;
      
        // QwtPlotZoomer
        bool accept( QPolygon & ) const override;
        QSizeF minZoomSize() const override;
      
        // QwtPicker
        void drawRubberBand( QPainter * ) const override;
        QwtText trackerTextF( const QPointF &pos ) const override;
        
    private slots:
        void handleZoomed( const QRectF& );
    };

}

#endif // ZOOMER_H
