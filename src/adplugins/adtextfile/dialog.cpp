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

#include "dialog.hpp"
#include "ui_dialog.h"
#include <QStandardItemModel>
#include <adportable/debug.hpp>
#include <ratio>

using namespace adtextfile;

Dialog::Dialog(QWidget *parent) : QDialog(parent)
                                , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->tableView->setModel( new QStandardItemModel );

    connect( ui->radioButton_6, &QRadioButton::toggled, this, [&](bool f){
            ui->groupBox_3->setEnabled( !f );
        });
}

Dialog::~Dialog()
{
    delete ui;
}

void
Dialog::setDataType( data_type t )
{
    ui->radioButton->setChecked( t == data_chromatogram );
    ui->radioButton_2->setChecked( t == data_spectrum );
    ui->radioButton_6->setChecked( t == counting_time_data );
}

Dialog::data_type
Dialog::dataType() const
{
    if ( ui->radioButton->isChecked() )
        return data_chromatogram;
    else if ( ui->radioButton_2->isChecked() )
        return data_spectrum;
    else
        return counting_time_data;
}

void
Dialog::setScanLaw( const std::string& adfsname
                    , double acclVoltage
                    , double tDelay
                    , double fLength
                    , const std::string& spectrometer )
{
    ui->groupBox_2->setEnabled( true );
    ui->groupBox_2->setChecked( true );
    ui->groupBox_2->setTitle( QString( tr( "Scan Law (%1)" ).arg( QString::fromStdString( spectrometer ) ) ) );

    ui->doubleSpinBox->setValue( fLength );
    ui->doubleSpinBox_2->setValue( acclVoltage );
    ui->doubleSpinBox_3->setValue( tDelay * std::micro::den );
}

void
Dialog::setAcceleratorVoltage( double v )
{
    ui->doubleSpinBox_2->setValue( v );
}

double
Dialog::acceleratorVoltage() const
{
    return ui->doubleSpinBox_2->value();
}

void
Dialog::setLength( double v )
{
    return ui->doubleSpinBox->setValue( v );
}

double
Dialog::length() const
{
    return ui->doubleSpinBox->value();
}

void
Dialog::setHasDataInterpreer( bool v )
{
    ui->checkBox->setChecked( v );
}

bool
Dialog::hasDataInterpreter() const
{
    return ui->checkBox->isChecked();
}

void
Dialog::setDataInterpreterClsids( const QStringList& list )
{
    ui->comboBox->addItems( list );
}

QString
Dialog::dataInterpreterClsid() const
{
    return ui->comboBox->currentText();
}

bool
Dialog::invertSignal() const
{
    return ui->checkBox_2->isChecked();
}

bool
Dialog::correctBaseline() const
{
    return ui->checkBox_3->isChecked();
}

adcontrols::metric::prefix
Dialog::dataPrefix() const
{
    return adcontrols::metric::prefix( (-3) * ui->comboBox_2->currentIndex() );
    // seconds           0
    // milliseconds     -3
    // microseconds     -6
    // nanoseconds      -9
    // pico             -12
}

void
Dialog::appendLine( const QStringList& list )
{
    size_t number_counts(0);
    int row;
    
    if ( auto model = static_cast<QStandardItemModel *>( ui->tableView->model() ) ) {
        model->setColumnCount( list.size() );
        row = model->rowCount();
        model->insertRow( row );
        int col = 0;
        for ( auto& data : list ) {
            model->setData( model->index( row, col++ ), data );
            bool ok( false );
            data.toDouble( &ok );
            if ( ok )
                number_counts++;
        }
        ui->tableView->resizeRowsToContents();
        ui->tableView->resizeColumnsToContents();
    }

    if ( number_counts != list.size() ) {
        ui->spinBox->setValue( row + 1 );
    }
    
    if ( number_counts >= 3 ) {
        // data has triplets (time, mass, intensity)
        ui->radioButton_3->setEnabled( false );
        ui->radioButton_4->setEnabled( false );
        ui->radioButton_5->setChecked( true );
        // 
        ui->doubleSpinBox->setEnabled( false );   // flight length
        ui->doubleSpinBox_2->setEnabled( false ); // accelerator voltage
    }
}

void
Dialog::setIsCentroid( bool f )
{
    ui->checkBox_4->setChecked( f );
}

bool
Dialog::isCentroid() const
{
    return ui->checkBox_4->isChecked();
}

bool
Dialog::isMassIntensity() const
{
    return ui->radioButton_4->isChecked();
}

bool
Dialog::isTimeMassIntensity() const
{
    return ui->radioButton_5->isChecked();
}

bool
Dialog::isTimeIntensity() const
{
    return ui->radioButton_3->isChecked();    
}

size_t
Dialog::columnCount() const
{
    if ( auto model = static_cast<QStandardItemModel *>( ui->tableView->model() ) )
        return model->columnCount();
    return 3;
}

size_t
Dialog::skipLines() const
{
    return ui->spinBox->value();
}
