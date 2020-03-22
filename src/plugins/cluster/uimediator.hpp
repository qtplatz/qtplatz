/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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
** Referencies;
** http://codingexodus.blogspot.jp/2012/12/working-with-video-using-opencv-and-qt.html
** http://codingexodus.blogspot.co.uk/2013/05/working-with-video-using-opencv-and-qt.html
**************************************************************************/

#pragma once

#include <QObject>

namespace cluster {

    enum ZMapId : int { zMapManip, zMapRaw };

    class uiMediator : public QObject {
        Q_OBJECT

        uiMediator(QObject *parent = 0);
    public:
        static uiMediator * instance();
        ~uiMediator();

    public slots:
        // select rect on spectrum view
        void tofRangeSelected( int id, const QRectF& rc ) { emit onTofRangeSelected( id, rc ); } // on SpectrumWidget (left top)

        // MidStyledToolBar
        void cellRangeToggled( bool checked ) { emit onCellRangeToggled( checked ); } // QCheckBox on mid toolbar
        void tofRangeToggled( bool checked ) { emit onTofRangeToggled( checked ); }   // QCheckBox on mid toolbar
        void tofDelayChanged( double microseconds ) { emit onTofDelayChanged( microseconds ); }
        void tofWidthChanged( double microseconds ) { emit onTofWidthChanged( microseconds ); }

        // TopStyledToolBar
        void zAutoScaleToggled( ZMapId id, bool checked ) { emit onZAutoScaleToggled( id, checked ); } // [0 = imfilter'ed, 1 = qwt::Spectrogram]
        void zScaleChanged( ZMapId id, int value ) { emit onZScaleChanged( id, value ); } // [0 = imfilter'ed, 1 = qwt::Spectrogram]

        // HistogramMethodForm
        void histogramClearCycleToggled( bool checked ) { emit onHistogramClearCycleToggled( checked ); }
        void histogramClearCycleChanged( int value ) { emit onHistogramClearCycleChanged( value ); }

    signals:
        void onTofRangeSelected( int id, const QRectF& rc );  // SpectrumWidget on PlayerWnd

        void onCellRangeToggled( bool );
        void onTofRangeToggled( bool );   // QCheckBox on mid toolbar
        void onTofDelayChanged( double ); // QDoubleSpinBox on mid toolbar
        void onTofWidthChanged( double ); // QDoubleSpinBox on mid toolbar

        void onZAutoScaleToggled( ZMapId, bool );
        void onZScaleChanged( ZMapId, int value );

        void onHistogramClearCycleToggled( bool checked );
        void onHistogramClearCycleChanged( int value );

    private:

    };

}
