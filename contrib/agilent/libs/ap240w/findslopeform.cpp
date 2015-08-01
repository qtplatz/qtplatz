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

#include "findslopeform.hpp"
#include "ui_findslopeform.h"

findSlopeForm::findSlopeForm(QWidget *parent) :  QWidget(parent)
                                              , ui(new Ui::findSlopeForm)
                                              , channel_(0)
    
{
    ui->setupUi(this);

    connect( ui->groupBox, &QGroupBox::toggled, [this] ( bool on ) { emit toggled( channel_, on ); });

    // Threshold (mV)
    connect( ui->doubleSpinBox, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged )
             , [this]( double value ) { 
                 emit thresholdChanged( channel_, ui->doubleSpinBox->value() );
             } );    

    // SG-Filter
    connect( ui->checkBox, &QCheckBox::toggled
             , [this] ( bool on ) {
                 emit sgFilterChanged( channel_, on, ui->spinBox->value() );
             } );
    
    connect( ui->spinBox, static_cast<void( QSpinBox::* )( int )>( &QSpinBox::valueChanged )
             , [this] ( int value ) {
                 emit sgFilterChanged( channel_, bool( ui->checkBox->isChecked()), value );
             } );

}

findSlopeForm::~findSlopeForm()
{
    delete ui;
}

void
findSlopeForm::set( int id, bool on, double th )
{
    if ( id == 0 ) {
        ui->checkBox->setChecked( on );
        ui->doubleSpinBox->setValue( th );
    }
}

bool
findSlopeForm::get( int id, bool& on, double& th ) const
{
    if ( id == 0 ) {
        on = ui->checkBox->isChecked();
        th = ui->doubleSpinBox->value();
        return true;
    }
    return false;
}

void
findSlopeForm::setTitle( int ch, const QString& title )
{
    channel_ = ch;
    ui->groupBox->setTitle( title );
}

bool
findSlopeForm::isChecked() const
{
    return ui->groupBox->isChecked();
}

void
findSlopeForm::setChecked( bool on )
{
    ui->groupBox->setChecked( on );
}

void
findSlopeForm::set( const ap240::threshold_method& m )
{
}

void
findSlopeForm::get( ap240::threshold_method& m ) const
{
}

int
findSlopeForm::channel() const
{
    return channel_;
}

