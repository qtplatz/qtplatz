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

#include "imagewidget.hpp"
#include "document.hpp"
#include <adportable/debug.hpp>
#include <QBoxLayout>
#include <QEvent>
#include <QGraphicsView>
#include <QLabel>
#include <QPainter>
#include <QRect>
#include <QStyle>
#include <QtOpenGL>
#include <QWheelEvent>

class QPaintEvent;

using namespace video;

ImageWidget::ImageWidget( QWidget * parent ) : QWidget( parent )
                                             , matrix_( std::make_unique< QMatrix >() )
{
    graphicsView_ = new QGraphicsView();
    graphicsView_->installEventFilter( this );

    matrix_->scale( 1.0, 1.0 );

    graphicsView_->setRenderHint(QPainter::Antialiasing, false);
    graphicsView_->setDragMode(QGraphicsView::RubberBandDrag);
    graphicsView_->setOptimizationFlags(QGraphicsView::DontSavePainterState);
    graphicsView_->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    graphicsView_->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    QSize iconSize(size, size);

    auto layout = new QBoxLayout( QBoxLayout::TopToBottom, this );
    layout->addWidget( graphicsView_ );

    setupMatrix();
}

ImageWidget::~ImageWidget()
{
}

void
ImageWidget::setImage( const QImage& image )
{
    auto scene = new QGraphicsScene;
    graphicsView_->setScene( scene );
    scene->addPixmap( QPixmap::fromImage( image ) );
}

void
ImageWidget::zoom( int delta )
{
    ADDEBUG() << "zoom(" << delta << ")";
}

void
ImageWidget::setupMatrix()
{
    QMatrix matrix( *matrix_ );
    graphicsView_->setMatrix(matrix);
}

QGraphicsView *
ImageWidget::graphicsView()
{
    return graphicsView_;
}

bool
ImageWidget::eventFilter( QObject * object, QEvent * event )
{
    if ( object == graphicsView_ ) {
        if ( event->type() == QEvent::Wheel ) {
            auto e = static_cast< QWheelEvent * >(event);
            zoom( e->delta() > 0 ? 6 : -6 );
            event->accept();
        }
    }
    return false;
}


#include "imagewidget.moc"





