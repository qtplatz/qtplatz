/**************************************************************************
** Copyright (C) 2016-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
**************************************************************************/

#pragma once

#include "adcv_global.hpp"
#include <QFrame>
#include <QImage>
#include <memory>

class QGraphicsView;
class QPaintEvent;
class QImage;

namespace adcv {

    class GraphicsView;

    class ADCVSHARED_EXPORT ImageWidget : public QWidget {

        Q_OBJECT

    public:
        ImageWidget( QWidget * parent = 0 );
        ~ImageWidget();

        void setImage( const QImage& );
        QGraphicsView * graphicsView();

        void sync( ImageWidget * );

    protected:
        bool eventFilter( QObject *, QEvent * event ) override;
        void zoom( int delta );

    public slots:
        void handleZoom( double scale );

    signals:
        void onZoom( double sclae );

    private slots:
        void setupMatrix();

    protected:

    private:
        QGraphicsView *graphicsView_;
        double scale_;
        size_t width_;
        size_t height_;
    };
}
