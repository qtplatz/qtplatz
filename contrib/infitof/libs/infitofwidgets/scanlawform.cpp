/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "scanlawform.hpp"
#include "ui_scanlawform.h"
#include <QSignalBlocker>

using namespace infitofwidgets;

ScanLawForm::ScanLawForm(QWidget *parent) : QWidget(parent)
                                          , ui(new Ui::ScanLawForm)
{
    ui->setupUi(this);

    connect( ui->spinBox
             , static_cast< void(QSpinBox::*)(int) >(&QSpinBox::valueChanged), this, [&]( int ){ emit valueChanged( idNLaps ); } );

    connect( ui->doubleSpinBox_TDelay
             , static_cast< void(QDoubleSpinBox::*)(double) >(&QDoubleSpinBox::valueChanged), this, [&]( double ){ emit valueChanged( idTDelay ); } );

    connect( ui->doubleSpinBox_AcclVoltage
             , static_cast< void(QDoubleSpinBox::*)(double) >(&QDoubleSpinBox::valueChanged), this, [&]( double ){ emit valueChanged( idAcclVoltage ); } );

    connect( ui->doubleSpinBox_LinearLength
             , static_cast< void(QDoubleSpinBox::*)(double) >(&QDoubleSpinBox::valueChanged), this, [&]( double ){ emit valueChanged( idLinearLength ); } );
}

ScanLawForm::~ScanLawForm()
{
    delete ui;
}

void
ScanLawForm::setLengthPrecision( int prec )
{
    QSignalBlocker block( this );
    ui->doubleSpinBox_LinearLength->setDecimals( prec );
    ui->doubleSpinBox_OrbitalLength->setDecimals( prec );
    ui->doubleSpinBox_L1->setDecimals( prec );
}

void
ScanLawForm::setLinearLength( double value )
{
    QSignalBlocker block( this );
    ui->doubleSpinBox_LinearLength->setValue( value );
}

void
ScanLawForm::setOrbitalLength( double value )
{
    QSignalBlocker block( this );
    ui->doubleSpinBox_OrbitalLength->setValue( value );
}

void 
ScanLawForm::setL1( double length, bool original )
{
    QSignalBlocker block( this );
    ui->doubleSpinBox_L1->setValue( length );
    if ( original )
        ui->doubleSpinBox_L10->setValue( length );
}

void
ScanLawForm::setAcceleratorVoltage( double value, bool original )
{
    QSignalBlocker block( this );
    ui->doubleSpinBox_AcclVoltage->setValue( value );
    if ( original )
        ui->doubleSpinBox_AcclVoltage0->setValue( value );
}

void
ScanLawForm::setTDelay( double value, bool original )
{
    QSignalBlocker block( this );
    ui->doubleSpinBox_TDelay->setValue( value );
    if ( original )
        ui->doubleSpinBox_TDelay0->setValue( value );
}
        
void
ScanLawForm::setNlaps( int n )
{
    QSignalBlocker block( this );
    ui->spinBox->setValue( n );
}

double
ScanLawForm::linearLength() const
{
    return ui->doubleSpinBox_LinearLength->value();
}

double
ScanLawForm::orbitalLength() const
{
    return ui->doubleSpinBox_OrbitalLength->value();
}

double
ScanLawForm::acceleratorVoltage() const
{
    return ui->doubleSpinBox_AcclVoltage->value();
}

double
ScanLawForm::tDelay() const
{
    return ui->doubleSpinBox_TDelay->value();
}

double
ScanLawForm::L1() const
{
    return ui->doubleSpinBox_L1->value();    
}
