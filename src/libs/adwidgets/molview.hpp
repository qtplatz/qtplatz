/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#pragma once

#include "adwidgets_global.hpp"
#include <QGraphicsView>
#include <memory>

QT_BEGIN_NAMESPACE
class QGraphicsSvgItem;
class QSvgRenderer;
class QWheelEvent;
class QPaintEvent;
QT_END_NAMESPACE

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT MolView : public QGraphicsView {
    public:
        explicit MolView( QWidget * parent = nullptr );

    public slots:
        void setHighQualityAntialiasing( bool highQualityAntialiasing );
        void setViewBackground( bool enable );
        void setViewOutline( bool enable );
        void drawBackground( QPainter *p, const QRectF &rect ) override;
        QSize svgSize() const;
        QSvgRenderer *renderer() const;
        bool setData( const QVariant& );
        
    protected:
        void wheelEvent(QWheelEvent *event) override;
        void paintEvent(QPaintEvent *event) override;
        
    private:
        std::unique_ptr< QSvgRenderer > renderer_;
        QGraphicsSvgItem * svgItem_;
        QGraphicsRectItem * backgroundItem_;
        QGraphicsRectItem * outlineItem_;
    };
}


