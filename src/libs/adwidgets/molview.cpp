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

#include "molview.hpp"

using namespace adwidgets;

#include <QSvgRenderer>

#include <QWheelEvent>
#include <QMouseEvent>
#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>
#include <QPaintEvent>
#include <qmath.h>

#ifndef QT_NO_OPENGL
#include <QGLWidget>
#endif

MolView::MolView(QWidget *parent) : QGraphicsView(parent)
                                  , svgItem_(nullptr)
                                  , backgroundItem_(nullptr)
                                  , outlineItem_(nullptr)
{
    setScene(new QGraphicsScene(this));
    setTransformationAnchor(AnchorUnderMouse);
    setDragMode(ScrollHandDrag);
    setViewportUpdateMode(FullViewportUpdate);

    // Prepare background check-board pattern
    QPixmap tilePixmap(64, 64);
    tilePixmap.fill(Qt::white);
    QPainter tilePainter(&tilePixmap);
    QColor color(220, 220, 220);
    tilePainter.fillRect(0, 0, 32, 32, color);
    tilePainter.fillRect(32, 32, 32, 32, color);
    tilePainter.end();

    setBackgroundBrush(tilePixmap);
}

void
MolView::drawBackground(QPainter *p, const QRectF &)
{
    p->save();
    p->resetTransform();
    p->drawTiledPixmap(viewport()->rect(), backgroundBrush().texture());
    p->restore();
}

QSize
MolView::svgSize() const
{
    return svgItem_ ? svgItem_->boundingRect().size().toSize() : QSize();
}

#if 0
bool
MolView::openFile( const QString &fileName )
{
    QGraphicsScene *s = scene();

    const bool drawBackground = (backgroundItem ? backgroundItem->isVisible() : false);
    const bool drawOutline = (outlineItem ? outlineItem->isVisible() : true);

    QScopedPointer<QGraphicsSvgItem> svgItem(new QGraphicsSvgItem(fileName));
    if (!svgItem->renderer()->isValid())
        return false;

    s->clear();
    resetTransform();

    svgItem = svgItem.take();
    svgItem->setFlags(QGraphicsItem::ItemClipsToShape);
    svgItem->setCacheMode(QGraphicsItem::NoCache);
    svgItem->setZValue(0);

    backgroundItem = new QGraphicsRectItem(svgItem->boundingRect());
    backgroundItem->setBrush(Qt::white);
    backgroundItem->setPen(Qt::NoPen);
    backgroundItem->setVisible(drawBackground);
    backgroundItem->setZValue(-1);

    outlineItem = new QGraphicsRectItem(svgItem->boundingRect());
    QPen outline(Qt::black, 2, Qt::DashLine);
    outline.setCosmetic(true);
    outlineItem->setPen(outline);
    outlineItem->setBrush(Qt::NoBrush);
    outlineItem->setVisible(drawOutline);
    outlineItem->setZValue(1);

    s->addItem(backgroundItem);
    s->addItem(svgItem);
    s->addItem(outlineItem);

    s->setSceneRect(outlineItem->boundingRect().adjusted(-10, -10, 10, 10));
    return true;
}
#endif

#if 0
void MolView::setRenderer(RendererType type)
{
    renderer = type;

    if (renderer == OpenGL) {
#ifndef QT_NO_OPENGL
        setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
#endif
    } else {
        setViewport(new QWidget);
    }
}
#endif

void MolView::setHighQualityAntialiasing(bool highQualityAntialiasing)
{
#ifndef QT_NO_OPENGL
    setRenderHint(QPainter::HighQualityAntialiasing, highQualityAntialiasing);
#else
    Q_UNUSED(highQualityAntialiasing);
#endif
}

void
MolView::setViewBackground( bool enable )
{
    if ( ! backgroundItem_ )
          return;

    backgroundItem_->setVisible( enable );
}

void
MolView::setViewOutline( bool enable )
{
    if ( ! outlineItem_ )
        return;

    outlineItem_->setVisible( enable );
}

void
MolView::paintEvent( QPaintEvent *event )
{
    QGraphicsView::paintEvent(event);
}

void
MolView::wheelEvent(QWheelEvent *event)
{
    qreal factor = qPow(1.2, event->delta() / 240.0);
    scale(factor, factor);
    event->accept();
}

QSvgRenderer*
MolView::renderer() const
{
    if ( svgItem_ )
        return svgItem_->renderer();
    
    return nullptr;
}
