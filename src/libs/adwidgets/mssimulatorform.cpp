/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mssimulatorform.hpp"
#include "ui_mssimulatorform.h"
#include <adcontrols/mssimulatormethod.hpp>
#include <QSignalBlocker>

using namespace adwidgets;

MSSimulatorForm::MSSimulatorForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MSSimulatorForm)
{
    ui->setupUi(this);
    connect( ui->spinBox, static_cast<void( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), [this] ( int ) { emit onValueChanged(); } );
    connect( ui->spinBox_2, static_cast<void( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), [this] ( int ) { emit onValueChanged(); } );
    connect( ui->spinBox_3, static_cast<void( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), [this] ( int ) { emit onValueChanged(); } );
    connect( ui->doubleSpinBox, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), [this] ( double ) { emit onValueChanged(); } );
    connect( ui->doubleSpinBox_2, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), [this] ( double ) { emit onValueChanged(); } );
    connect( ui->doubleSpinBox_3, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), [this] ( double ) { emit onValueChanged(); } );
    connect( ui->doubleSpinBox_4, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), [this] ( double ) { emit onValueChanged(); } );
    connect( ui->doubleSpinBox_4, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), [this] ( double ) { emit onValueChanged(); } );
    connect( ui->checkBox, &QCheckBox::toggled, [this](bool) { emit onValueChanged(); } );
    connect( ui->groupBox, &QGroupBox::toggled, [this](bool) { emit onValueChanged(); } );    
    connect( ui->pushButton, &QPushButton::pressed, [this] () { emit onProcess(); } );

    ui->doubleSpinBox_5->setMinimum( -1000.0 ); // us
    ui->doubleSpinBox_5->setMaximum( 1000.0 ); // us
}

MSSimulatorForm::~MSSimulatorForm()
{
    delete ui;
}

void
MSSimulatorForm::OnInitialUpdate()
{
}

void
MSSimulatorForm::OnFinalClose()
{
}

bool
MSSimulatorForm::getContents( adcontrols::MSSimulatorMethod& m ) const
{
    m.setResolvingPower( ui->spinBox->value() );
    m.setLMassLimit( ui->checkBox->isChecked() ? ui->doubleSpinBox->value() : -ui->doubleSpinBox->value() );
    m.setUMassLimit( ui->checkBox_2->isChecked() ? ui->doubleSpinBox_2->value() : -ui->doubleSpinBox_2->value() );
    m.setChargeStateMin( ui->spinBox_2->value() );
    m.setChargeStateMax( ui->spinBox_3->value() );
    m.setIsTof( ui->groupBox->isChecked() );
    m.setLength( ui->doubleSpinBox_3->value() );
    m.setAcceleratorVoltage( ui->doubleSpinBox_4->value() );
    m.setTDelay( ui->doubleSpinBox_5->value() * 1.0e-6 );
    
    return true;
}

bool
MSSimulatorForm::setContents( const adcontrols::MSSimulatorMethod& m )
{
    QSignalBlocker blocks[] = { QSignalBlocker( ui->spinBox )
                                , QSignalBlocker( ui->spinBox_2 )
                                , QSignalBlocker( ui->spinBox_3 )
                                , QSignalBlocker( ui->checkBox )
                                , QSignalBlocker( ui->groupBox )
                                , QSignalBlocker( ui->doubleSpinBox )
                                , QSignalBlocker( ui->doubleSpinBox_2 )
                                , QSignalBlocker( ui->doubleSpinBox_3 )
                                , QSignalBlocker( ui->doubleSpinBox_4 )
                                , QSignalBlocker( ui->doubleSpinBox_5 ) };

    ui->spinBox->setValue( m.resolvingPower() );
    ui->checkBox->setChecked( m.lMassLimit() > 0 );    
    ui->doubleSpinBox->setValue( m.lMassLimit() > 0 ? m.lMassLimit() : -m.lMassLimit() );
    ui->doubleSpinBox_2->setValue( m.uMassLimit() > 0 ? m.uMassLimit() : -m.uMassLimit() );
    
    ui->spinBox_2->setValue(  m.chargeStateMin() );
    ui->spinBox_3->setValue( m.chargeStateMax() );
    ui->groupBox->setChecked ( m.isTof() );
    ui->doubleSpinBox_3->setValue( m.length() );
    ui->doubleSpinBox_4->setValue( m.acceleratorVoltage() );
    ui->doubleSpinBox_5->setValue( m.tDelay() * 1.0e6 );
    
    return true;
}

