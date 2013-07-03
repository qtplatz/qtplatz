/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#ifndef BEZELWIDGET_HPP
#define BEZELWIDGET_HPP

#include <QWidget>

class QGraphicsScene;
class QGraphicsPixmapItem;
class QGraphicsView;

namespace Ui {
class BezelWidget;
}

namespace toftune {

    class BezelWidget : public QWidget {
        Q_OBJECT
    
    public:
        explicit BezelWidget(QWidget *parent = 0);
        ~BezelWidget();
    
    private:
        Ui::BezelWidget *ui;
        QPixmap * pixmap_;
        QGraphicsPixmapItem * item_;
        QGraphicsScene * scene_;

    protected:
        void resizeEvent( QResizeEvent * );
    };

}

#endif // BEZELWIDGET_HPP
