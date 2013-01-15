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

#include "sequencesform.hpp"
#include "ui_sequencesform.h"
#include "sequencesmodel.hpp"
#include <QStandardItemModel>

using namespace qtwidgets;

SequencesForm::SequencesForm(QWidget *parent) : QWidget(parent)
                                              , ui(new Ui::SequencesForm)
                                              , pModel_( new QStandardItemModel ) 
                                              // , pModel_( new qtwidgets::SequencesModel )
{
    ui->setupUi(this);
    ui->tableView->setModel( pModel_.get() );
}

SequencesForm::~SequencesForm()
{
    delete ui;
}

void
SequencesForm::OnCreate( const adportable::Configuration& config )
{
    config_ = config;
}

void
SequencesForm::OnInitialUpdate()
{
    QStandardItemModel& model = *pModel_;
    QStandardItem * rootNode = model.invisibleRootItem();

    ui->tableView->setRowHeight( 0, 10 );
    rootNode->setColumnCount(10);
    int col = 0;
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Sample Type") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Vial") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Description") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Inj.Vol(uL)") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Method Time(min)") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Dataset Name") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Control Method") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Process Method") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Target Formulae") );

    if ( model.rowCount() == 0 ) {
        rootNode->appendRow( new QStandardItem( "STD/QC/UNK" ) );
    }
}

void
SequencesForm::OnFinalClose()
{
}

bool
SequencesForm::getContents( boost::any& ) const
{
    return false;
}

bool
SequencesForm::setContents( boost::any& )
{
    return false;
}


void
SequencesForm::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = static_cast< adplugin::LifeCycle *>(this);
}

