// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "sequencewidget.hpp"
#include "ui_sequencewidget.h"
#include <adportable/configuration.hpp>
#include <QStandardItemModel>

using namespace qtwidgets;

SequenceWidget::SequenceWidget(QWidget *parent) : QWidget(parent)
                                                , ui(new Ui::SequenceWidget)
                                                , pModel_( new QStandardItemModel )
                                                , pConfig_( new adportable::Configuration )
{
    ui->setupUi(this);
    ui->treeView->setModel( pModel_.get() );
}

SequenceWidget::~SequenceWidget()
{
    delete ui;
}

void
SequenceWidget::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;
}

void
SequenceWidget::OnInitialUpdate()
{
}

void
SequenceWidget::OnFinalClose()
{
}

void
SequenceWidget::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = static_cast< adplugin::LifeCycle *>(this);
}
