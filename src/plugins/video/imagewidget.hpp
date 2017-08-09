/**************************************************************************
** Copyright (C) 2016-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <QFrame>
#include <QImage>
#include <memory>

class QGraphicsView;
class QPaintEvent;
class QImage;

namespace video {

    class ImageWidget : public QWidget {

        Q_OBJECT

    public:
        ImageWidget( QWidget * parent = 0 );
        ~ImageWidget();

        void setImage( const QImage& );
        QGraphicsView * graphicsView();

    protected:
        bool eventFilter( QObject *, QEvent * event ) override;
        void zoom( int delta );
    
    public slots:

    private slots:
        void setupMatrix();

    protected:
    
    private:
        QGraphicsView *graphicsView_;
        std::unique_ptr< QMatrix > matrix_;
    };
}

