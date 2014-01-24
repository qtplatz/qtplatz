/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#ifndef SPECTROGRAMWIDGET_HPP
#define SPECTROGRAMWIDGET_HPP

#include <qwt_plot.h>
#include <memory>

class QwtPlotSpectrogram;
class QwtRasterData;
class QwtPlotZoomer;
class QwtPlotPanner;

namespace adwplot {

    class SpectrogramData;

    class SpectrogramWidget : public QwtPlot {
        Q_OBJECT
    public:
        explicit SpectrogramWidget(QWidget *parent = 0);
        void setData( SpectrogramData * );
    
    signals:
        void dataChanged();
                          
    public slots:
        void handleShowContour( bool on );
        void handleShowSpectrogram( bool on );
        void handleSetAlpha( int );
        void handleDataChanged();
    private slots:
        void handleZoomed( const QRectF& );
        
    private:
        QwtPlotSpectrogram * spectrogram_;
        QwtPlotZoomer * zoomer_;
        QwtPlotPanner * panner_;
        SpectrogramData * data_;
        void handle_signal();

        QwtText tracker1( const QPointF& );
        QwtText tracker2( const QPointF&, const QPointF& );
    };

}

#endif // SPECTROGRAMWIDGET_HPP
