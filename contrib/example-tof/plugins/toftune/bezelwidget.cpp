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

#include "bezelwidget.hpp"
#include "ui_bezelwidget.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QResizeEvent>

using namespace toftune;

BezelWidget::BezelWidget(QWidget *parent) : QWidget(parent)
                                          , ui(new Ui::BezelWidget)
                                          , pixmap_( new QPixmap( ":/toftune/images/IMG_0050.png" ) )
                                          , item_( new QGraphicsPixmapItem )
                                          , scene_( new QGraphicsScene )
{
    ui->setupUi(this);

    QPixmap scaled = pixmap_->scaled( size() * 0.8, Qt::KeepAspectRatio, Qt::SmoothTransformation );
    item_->setPixmap( scaled );
    scene_->addItem( item_ );
    ui->graphicsView->setScene( scene_ );
}

BezelWidget::~BezelWidget()
{
    delete pixmap_;
    delete item_;
    delete scene_;
    delete ui;
}

void
BezelWidget::resizeEvent( QResizeEvent * ev )
{
    QPixmap scaled = pixmap_->scaled( ev->size() * 0.8, Qt::KeepAspectRatio, Qt::SmoothTransformation );
    item_->setPixmap( scaled );

    QWidget::resizeEvent( ev );
}