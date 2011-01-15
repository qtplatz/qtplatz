/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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

#include "targetingform.h"
#include "ui_targetingform.h"
#include "targetingdelegate.h"
#include <adcontrols/targetingmethod.h>
#include <adcontrols/processmethod.h>
#include <adportable/configuration.h>
#pragma warning(disable:4251)
#include <QStandardItemModel>
#pragma warning(default:4251)

using namespace qtwidgets;

TargetingForm::TargetingForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TargetingForm)
    , pModel_( new QStandardItemModel )
    , pConfig_( new adportable::Configuration )
    , pMethod_( new adcontrols::TargetingMethod )
    , pDelegate_( new TargetingDelegate ) 
{
    ui->setupUi(this);
    ui->treeView->setModel( pModel_.get() );
    ui->treeView->setItemDelegate( pDelegate_.get() );
}

TargetingForm::~TargetingForm()
{
    delete ui;
}

void
TargetingForm::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;
}

void
TargetingForm::OnInitialUpdate()
{
    QStandardItemModel& model = *pModel_;
    // adcontrols::TargetingMethod& method = *pMethod_;

    QStandardItem * rootNode = model.invisibleRootItem();

    ui->treeView->setItemDelegate( pDelegate_.get() );

    rootNode->setColumnCount(2);
    model.setHeaderData( 0, Qt::Horizontal, "Targeting" );
}

void
TargetingForm::OnFinalClose()
{
}

void
TargetingForm::getContents( adcontrols::ProcessMethod& pm )
{
    pm.appendMethod< adcontrols::TargetingMethod >( *pMethod_ );
}