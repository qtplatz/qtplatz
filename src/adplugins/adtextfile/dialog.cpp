/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <app/app_version.h>
#include <QDebug>

using namespace adtextfile;

Dialog::Dialog(QWidget *parent) : QDialog(parent)
                                , ui(new Ui::Dialog)
                                , settings_( QSettings::IniFormat
                                             , QSettings::UserScope
                                             , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                             , QLatin1String( "adtextfile" ) )
                                , nColumns_( 0 )
{
    ui->setupUi(this);
    ui->tableView->setModel( new QStandardItemModel );

    connect( ui->radioButton_6, &QRadioButton::toggled, this, [&](bool f){
            ui->groupBox_3->setEnabled( !f );
        });

    connect( ui->lineEdit, &QLineEdit::textChanged, this, [&]( const QString& text ){
            QRegExp sep( "(\\ |\\,|\\t)"); // ' '|','|\t
            QStringList list = text.split( sep );
            QSet< int > lines;
            for ( const auto& str: list ) {
                bool ok( false );
                int line = str.toInt( &ok );
                if ( ok && line > 0 )   // origin one for column
                    lines << line;
            }
            setColumnsIgnored( lines );
        });

    settings_.beginGroup( "Dialog" );
    ui->checkBox->setChecked( settings_.value( "CheckBoxFileCreatedBy", true ).toBool() );
    setDataType( static_cast< data_type >( settings_.value( "RadioButtonDataType", data_spectrum ).toInt() ) ); 
    ui->checkBox_4->setChecked( settings_.value( "CheckBoxPeakList", false ).toBool() );
    auto data_pair = settings_.value( "RadioButtonDataPair", "Time,Intensity" ).toString();
    if ( data_pair == "Time,Intensity" )
        ui->radioButton_3->setChecked( true );
    if ( data_pair == "Mass,Intensity" )
        ui->radioButton_4->setChecked( true );

    ui->checkBox_2->setChecked( settings_.value( "CheckBoxInverseData", true ).toBool() );
    ui->checkBox_3->setChecked( settings_.value( "CheckBoxBaselineCorrection", false ).toBool() );

    ui->comboBox_2->setCurrentIndex( settings_.value( "ComboBoxTimePrefix", 2 ).toInt() ); // microseconds
    ui->spinBox->setValue( settings_.value( "SpinBoxSkipFirst", 0 ).toInt() );

    ui->lineEdit->setText( settings_.value( "ColumnsIgnored", QString() ).toString() );
    ui->groupBox_2->setChecked( settings_.value( "ScanLawBox", true ).toBool() );
    ui->doubleSpinBox->setValue( settings_.value( "ScanLaw/Length", 1.0 ).toDouble() );
    ui->doubleSpinBox_2->setValue( settings_.value( "ScanLaw/Accelerator", 5000 ).toDouble() );
    ui->doubleSpinBox_3->setValue( settings_.value( "ScanLaw/T0", 0.0 ).toDouble() );

    settings_.endGroup();

}

Dialog::~Dialog()
{
    settings_.beginGroup( "Dialog" );

    settings_.setValue( "CheckBoxFileCreatedBy", ui->checkBox->isChecked() );
    settings_.setValue( "RadioButtonDataType", static_cast<int>( dataType() ) );
    settings_.setValue( "CheckBoxPeakList", ui->checkBox_4->isChecked() );
    if ( ui->radioButton_3->isChecked() )
        settings_.setValue( "RadioButtonDataPair", "Time,Intensity" );
    if ( ui->radioButton_4->isChecked() )
        settings_.setValue( "RadioButtonDataPair", "Mass,Intensity" );

    settings_.setValue( "CheckBoxInverseData", ui->checkBox_2->isChecked() );
    settings_.setValue( "CheckBoxBaselineCorrection", ui->checkBox_3->isChecked() );
    settings_.setValue( "ComboBoxTimePrefix", ui->comboBox_2->currentIndex() );
    settings_.setValue( "SpinBoxSkipFirst", ui->spinBox->value() );
    QString list;
    for ( auto& i: ignoreColumns_ )
        list += QString( ( list.isEmpty() ? "%1" : ", %1" ) ).arg( i );
    settings_.setValue( "ColumnsIgnored", list );

    settings_.setValue( "ScanLawBox", ui->groupBox_2->isChecked() );
    settings_.setValue( "ScanLaw/Length", ui->doubleSpinBox->value() );
    settings_.setValue( "ScanLaw/Accelerator", ui->doubleSpinBox_2->value() );
    settings_.setValue( "ScanLaw/T0", ui->doubleSpinBox_3->value() );

    settings_.endGroup();
    
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
Dialog::setScanLaw( double acclVoltage
                    , double tDelay
                    , double fLength
                    , const QString& spectrometer )
{
    ui->groupBox_2->setEnabled( true );
    ui->groupBox_2->setChecked( true );
    ui->groupBox_2->setTitle( QString( tr( "Scan Law (%1)" ).arg( spectrometer ) ) );

    ui->doubleSpinBox->setValue( fLength );
    ui->doubleSpinBox_2->setValue( acclVoltage );
    ui->doubleSpinBox_3->setValue( tDelay * std::micro::den );
}

bool
Dialog::hasScanLaw() const
{
    return ui->groupBox_2->isChecked();
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
Dialog::setTDelay( double v )
{
    ui->doubleSpinBox_3->setValue( v );
}

double
Dialog::tDelay() const
{
    return ui->doubleSpinBox_3->value();
}

void
Dialog::setHasDataInterpreter( bool v )
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

    nColumns_ = number_counts;
    if ( ignoreColumns_.size() < nColumns_ )
        setActiveColumns( number_counts - ignoreColumns_.size() );
    else
        setActiveColumns( number_counts );
}

void
Dialog::setActiveColumns( size_t nColumns )
{
    if ( nColumns >= 3 ) {
        // data has triplets (time, mass, intensity)
        ui->radioButton_3->setEnabled( false );
        ui->radioButton_4->setEnabled( false );
        ui->radioButton_5->setChecked( true );
        // 
        ui->doubleSpinBox->setEnabled( false );   // flight length
        ui->doubleSpinBox_2->setEnabled( false ); // accelerator voltage
    } else {
        ui->radioButton_3->setEnabled( true );
        ui->radioButton_4->setEnabled( true );
        ui->radioButton_5->setChecked( false );
        // 
        ui->doubleSpinBox->setEnabled( true );   // flight length
        ui->doubleSpinBox_2->setEnabled( true ); // accelerator voltage        
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

void
Dialog::setColumnsIgnored( const QSet< int >& lines )
{
    ignoreColumns_ = lines;
    if ( nColumns_ > ignoreColumns_.size() )
        setActiveColumns( nColumns_ - ignoreColumns_.size() );
}

const QSet< int >&
Dialog::ignoreColumns() const
{
    return ignoreColumns_;
}

