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
    m.set_resolving_power( ui->spinBox->value() );
    m.set_lower_limit( ui->checkBox->isChecked() ? ui->doubleSpinBox->value() : -ui->doubleSpinBox->value() );
    m.set_upper_limit( ui->checkBox_2->isChecked() ? ui->doubleSpinBox_2->value() : -ui->doubleSpinBox_2->value() );
    m.set_charge_state_min( ui->spinBox_2->value() );
    m.set_charge_state_max( ui->spinBox_3->value() );
    m.set_is_tof( ui->groupBox->isChecked() );
    m.set_length( ui->doubleSpinBox_3->value() );
    m.set_accelerator_voltage( ui->doubleSpinBox_4->value() );
    m.set_tDelay( ui->doubleSpinBox_5->value() );
    
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

    ui->spinBox->setValue( m.resolving_power() );
    ui->checkBox->setChecked( m.lower_limit() > 0 );    
    ui->doubleSpinBox->setValue( m.lower_limit() > 0 ? m.lower_limit() : -m.lower_limit() );
    ui->doubleSpinBox_2->setValue( m.upper_limit() > 0 ? m.upper_limit() : -m.upper_limit() );    
    
    ui->spinBox_2->setValue(  m.charge_state_min() );
    ui->spinBox_3->setValue( m.charge_state_max() );
    ui->groupBox->setChecked ( m.is_tof() );
    ui->doubleSpinBox_3->setValue( m.length() );
    ui->doubleSpinBox_4->setValue( m.accelerator_voltage() );
    ui->doubleSpinBox_5->setValue( m.tDelay() );
    
    return true;
}

