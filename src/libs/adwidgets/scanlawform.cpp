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

using namespace adwidgets;

ScanLawForm::ScanLawForm(QWidget *parent) : QWidget(parent)
                                          , ui(new Ui::ScanLawForm)
{
    ui->setupUi(this);
    int id = 0;
    for ( auto spin: { ui->doubleSpinBox, ui->doubleSpinBox_2, ui->doubleSpinBox_3 } ) {
        connect( spin, static_cast< void(QDoubleSpinBox::*)(double) >(&QDoubleSpinBox::valueChanged)
                 , this, [=]( double ){ emit valueChanged( id ); } );
        ++id;
    }
}

ScanLawForm::~ScanLawForm()
{
    delete ui;
}

void
ScanLawForm::setLengthPrecision( int prec )
{
    ui->doubleSpinBox->setDecimals( prec );
}

void
ScanLawForm::setLength( double value )
{
    QSignalBlocker block( ui->doubleSpinBox );
    ui->doubleSpinBox->setValue( value );
}

void
ScanLawForm::setAcceleratorVoltage( double value )
{
    QSignalBlocker block( ui->doubleSpinBox_2 );
    ui->doubleSpinBox_2->setValue( value );
}

void
ScanLawForm::setTDelay( double value )
{
    QSignalBlocker block( ui->doubleSpinBox_3 );    
    ui->doubleSpinBox_3->setValue( value );
}
        
double
ScanLawForm::length() const
{
    return ui->doubleSpinBox->value();
}

double
ScanLawForm::acceleratorVoltage() const
{
    return ui->doubleSpinBox_2->value();
}

double
ScanLawForm::tDelay() const
{
    return ui->doubleSpinBox_3->value();
}

