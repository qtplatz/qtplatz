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

#include "ap240triggerform.hpp"
#include "ui_ap240triggerform.h"
#include <ap240/digitizer.hpp>
#include <QSignalBlocker>

ap240TriggerForm::ap240TriggerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ap240TriggerForm)
{
    ui->setupUi(this);

    connect( ui->comboBox_2, static_cast<void( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), [this] ( int index ) {
            if ( index == 0 )
                emit valueChanged( idTrigPattern, QVariant( 0x80000000 ) );
            else 
                emit valueChanged( idTrigPattern, QVariant( index & 03 ) );
        });

    connect( ui->comboBox_3, static_cast<void( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), [this] ( int index ) {
            emit valueChanged( idTrigCoupling, QVariant( index  ) );
        });

    connect( ui->comboBox_4, static_cast<void( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), [this] ( int index ) {
            emit valueChanged( idTrigSlope, QVariant( index  ) );
        });    

    connect( ui->doubleSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](double d){
            emit valueChanged( idTrigLevel1, QVariant( d ) );
        });

    connect( ui->doubleSpinBox_2, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](double d){
            emit valueChanged( idTrigLevel2, QVariant( d ) );
        });    
}

ap240TriggerForm::~ap240TriggerForm()
{
    delete ui;
}

void
ap240TriggerForm::set( const ap240x::method& m )
{
    const QSignalBlocker blocker( this );

    if ( m.trig_.trigPattern & 0x80000000 )
        ui->comboBox_2->setCurrentIndex( 0 );
    else 
        ui->comboBox_2->setCurrentIndex( m.trig_.trigPattern & 03 );

    ui->comboBox_3->setCurrentIndex( m.trig_.trigClass );
    ui->comboBox_4->setCurrentIndex( m.trig_.trigSlope );
    ui->doubleSpinBox->setValue( m.trig_.trigLevel1 );
    ui->doubleSpinBox_2->setValue( m.trig_.trigLevel2 );
}

void
ap240TriggerForm::get( ap240x::method& m ) const
{
    if ( ui->comboBox_2->currentIndex() == 0 )
        m.trig_.trigPattern = 0x80000000;
    else
        m.trig_.trigPattern = ui->comboBox_2->currentIndex();
    m.trig_.trigClass = ui->comboBox_3->currentIndex();
    m.trig_.trigSlope = ui->comboBox_4->currentIndex();
    m.trig_.trigLevel1 = ui->doubleSpinBox->value();
    m.trig_.trigLevel2 = ui->doubleSpinBox_2->value();
}
