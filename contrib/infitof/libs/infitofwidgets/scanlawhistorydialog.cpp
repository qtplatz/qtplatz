/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "scanlawhistorydialog.hpp"
#include "ui_scanlawhistorydialog.h"
#include <adcontrols/msmoltable.hpp>
#include <adcontrols/mspeak.hpp>
#include <adwidgets/htmlheaderview.hpp>
#include <QItemSelectionModel>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>

using namespace infitofwidgets;

ScanLawHistoryDialog::ScanLawHistoryDialog(QWidget *parent) : QDialog(parent)
                                                            , ui(new Ui::ScanLawHistoryDialog)
{
    ui->setupUi(this);
    
    ui->masterView->setModel( new QSqlQueryModel() );
    ui->detailsView->setModel( new QSqlQueryModel() );

    ui->masterView->setHorizontalHeader( new adwidgets::HtmlHeaderView() );

    ui->masterView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->masterView->setSelectionMode(QAbstractItemView::SingleSelection);

    connect( ui->masterView->selectionModel(), &QItemSelectionModel::currentChanged, [&]( const QModelIndex& index, const QModelIndex& ){
            int id = index.model()->index( index.row(), 0 ).data().toInt();
            QSqlQuery query( "SELECT formula,nlaps,exactMass AS 'exact mass'"
                             ", mass,(mass-exactMass)*1000 AS 'Error(mDa)'"
                             ", time * 1e6 AS 'time(us)'"
                             ", width * 1e9 AS 'width(ns)'"
                             " FROM assigned WHERE id=?", *sqldb_ );
            query.addBindValue( id );            
            if ( !query.exec() )
                qDebug() << query.lastError();
            qobject_cast< QSqlQueryModel * >( ui->detailsView->model() )->setQuery( query );
            ui->detailsView->resizeColumnsToContents();
        });

    connect( ui->buttonBox, SIGNAL(accepted()), this, SLOT( accept() ) );
    connect( ui->buttonBox, SIGNAL(rejected()), this, SLOT( reject() ) );
}

ScanLawHistoryDialog::~ScanLawHistoryDialog()
{
    delete ui;
}

bool
ScanLawHistoryDialog::openDatabase( const QString& file )
{
    sqldb_ = std::make_unique< QSqlDatabase >( QSqlDatabase::addDatabase( "QSQLITE" ) );
    sqldb_->setDatabaseName( file );

    if ( sqldb_->open() ) {

        QSqlQuery query( "SELECT id, dateCreated AS 'Date & Time'"
                         ", accVoltage AS 'Accel. (V)'"
                         ", tDelay AS 'T<sub>0</sub>'"
                         ", idCreatedBy AS 'Operator'"
                         ", idComputer AS 'Computer'"
                         " FROM ident ORDER BY dateCreated", *sqldb_ );
        qobject_cast< QSqlQueryModel * >(ui->masterView->model() )->setQuery( query );
        ui->masterView->resizeColumnsToContents();

        if ( ui->masterView->model()->columnCount() > 0 )
            ui->masterView->setCurrentIndex( ui->masterView->model()->index( 0, 0 ) );
        
        return true;
    }
    return false;
}

std::shared_ptr< adcontrols::MSMolTable >
ScanLawHistoryDialog::selectedData()
{
    auto index = ui->masterView->currentIndex();
    if ( index.isValid() ) {
        QSqlQuery query( "SELECT data FROM ident WHERE id=?", *sqldb_ );
        query.addBindValue( index.model()->index( index.row(), 0 ).data().toInt() );
        if ( query.exec() && query.next() ) {
            auto blob = query.value( 0 ).toByteArray();
            auto table = std::make_shared< adcontrols::MSMolTable >();
            if ( adcontrols::MSMolTable::deserialize( *table, blob.constData(), blob.size() ) )
                // if ( adportable::binary::deserialize<>()( *table, blob.constData(), blob.size() ) )
                return table;
        } else {
            qDebug() << query.lastError();
        }
    }
    return nullptr;
}

