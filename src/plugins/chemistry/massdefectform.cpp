/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "massdefectform.hpp"
#include "ui_massdefectform.h"

#include "massdefectmethod.hpp"
#include "massdefectdelegate.hpp"
#include <adportable/configuration.hpp>
#include <qstandarditemmodel.h>
#include <qtwidgets/standarditemhelper.hpp>

using namespace chemistry;

MassDefectForm::MassDefectForm(QWidget *parent) : QWidget(parent)
	                                            , ui(new Ui::MassDefectForm)
												, model_( new QStandardItemModel )
												, method_( new MassDefectMethod )
												, delegate_( new MassDefectDelegate )
{
    ui->setupUi(this);
	ui->treeView->setModel( model_.get() );
    ui->treeView->setItemDelegate( delegate_.get() );
}

MassDefectForm::~MassDefectForm()
{
    delete ui;
}

void
MassDefectForm::OnCreate( const adportable::Configuration& )
{
}

void
MassDefectForm::OnInitialUpdate()
{
	QStandardItemModel& model = *model_;
    //MassDefectMethod& method = *method_;

    QStandardItem * rootNode = model.invisibleRootItem();
    rootNode->setColumnCount( 2 );
	model.setHeaderData( 0, Qt::Horizontal, "Mass defect method" );
    for ( int i = 1; i < rootNode->columnCount(); ++i )
		model.setHeaderData( 1, Qt::Horizontal, "" );

    using qtwidgets::StandardItemHelper;
	StandardItemHelper::appendRow( rootNode, "Polarity", "positive" );
  	StandardItemHelper::appendRow( rootNode, "Mass Tolerance[Da]", 0.1 );
    do {
		QStandardItem * adducts = new QStandardItem( "Adducts" );
		adducts->setEditable( false );
        rootNode->appendRow( adducts );
		StandardItemHelper::appendRow( adducts, "H", "H" );
	} while( 0 );

    do {
		QStandardItem * loss = new QStandardItem( "Loss" );
		loss->setEditable( false );
		rootNode->appendRow( loss );
	} while( 0 );
}


void
MassDefectForm::OnFinalClose()
{
}
