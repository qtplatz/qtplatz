/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "countingquerydialog.hpp"
#include "ui_countingquerydialog.h"
#include <QStandardItemModel>
#include <QTableView>
#include <QPushButton>

using namespace query;

CountingQueryDialog::CountingQueryDialog(QWidget *parent) : QDialog( parent, Qt::Tool )
                                                          , ui(new Ui::CountingQueryDialog)
{
    ui->setupUi(this);
    auto model = new QStandardItemModel;
    ui->tableView->setModel( model );
    model->setColumnCount( 4 );
    model->setHeaderData( 0, Qt::Horizontal, tr("name") );
    model->setHeaderData( 1, Qt::Horizontal, tr("protocol") );
    model->setHeaderData( 2, Qt::Horizontal, tr("lower limit(s)") );
    model->setHeaderData( 3, Qt::Horizontal, tr("upper limit(s)") );

    ui->comboBox->addItems( QStringList() << "COUNTING" << "COUNTING.FREQUENCY" << "ELAPSEDTIME.INTENSITY");

    if ( auto button = qobject_cast< QPushButton * >( ui->buttonBox->button( QDialogButtonBox::Reset ) ) )
        connect( button, &QPushButton::clicked, [this](){ clear(); } );

    if ( auto button = qobject_cast< QPushButton * >( ui->buttonBox->button( QDialogButtonBox::Apply ) ) )
        connect( button, &QPushButton::clicked, [this](){ emit applied(); } );

    connect( ui->buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
}

CountingQueryDialog::~CountingQueryDialog()
{
    delete ui;
}

QTableView *
CountingQueryDialog::tableView()
{
    return ui->tableView;
}

QAbstractItemModel *
CountingQueryDialog::model()
{
    return ui->tableView->model();
}

void
CountingQueryDialog::setCommandText( const QString& text )
{
    ui->comboBox->setCurrentText( text );
}

QString
CountingQueryDialog::commandText() const
{
    return ui->comboBox->currentText();
}

void
CountingQueryDialog::clear()
{
    qobject_cast< QStandardItemModel *>( model() )->setRowCount( 0 );
}

void
CountingQueryDialog::accept()
{
    emit accepted();
    hide();
}

