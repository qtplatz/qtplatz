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

#include "ap240horizontalform.hpp"
#include "ui_ap240horizontalform.h"
#include <ap240/digitizer.hpp>
#include <QSignalBlocker>

ap240HorizontalForm::ap240HorizontalForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ap240HorizontalForm)
{
    ui->setupUi(this);

    connect( ui->doubleSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](double d){
            emit valueChanged( idDelay, QVariant(d) );
        });

    connect( ui->doubleSpinBox_2, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](double d){
            emit valueChanged( idWidth, QVariant(d) );
        });
    
    connect( ui->comboBox, static_cast<void( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), [this] ( int index ) {
            emit valueChanged( idSampInterval, QVariant(index) );
        } );

    connect( ui->checkBox, &QCheckBox::stateChanged, [this] ( int state ) {
            emit valueChanged( idMode, QVariant( state == Qt::Checked ) );
        });

    connect( ui->spinBox, static_cast<void( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), [this] ( int d ) {
            emit valueChanged( idAvgWaveforms, QVariant(d) );
        });
}

ap240HorizontalForm::~ap240HorizontalForm()
{
    delete ui;
}

void
ap240HorizontalForm::set( const ap240::method& m )
{
    const QSignalBlocker blocker( this );
    
    ui->doubleSpinBox->setValue( m.hor_.delay );
    ui->doubleSpinBox_2->setValue( m.hor_.width );

    if ( m.hor_.sampInterval < 0.51e-9 )
        ui->comboBox->setCurrentIndex( 0 );
    else if ( m.hor_.sampInterval < 1.01e-9 )
        ui->comboBox->setCurrentIndex( 1 );
    else if ( m.hor_.sampInterval < 2.01e-9 )
        ui->comboBox->setCurrentIndex( 2 );

    ui->checkBox->setChecked( m.hor_.mode == 2 );
    ui->spinBox->setValue( m.hor_.nbrAvgWaveforms );
}

void
ap240HorizontalForm::get( ap240::method& m ) const
{
    m.hor_.delay = ui->doubleSpinBox->value();
    m.hor_.width = ui->doubleSpinBox_2->value();
    switch ( ui->comboBox->currentIndex() ) {
    case 0: 
        m.hor_.sampInterval = 0.5e-9;
        break;
    case 1:
        m.hor_.sampInterval = 1.0e-9;
        break;
    case 2:
        m.hor_.sampInterval = 2.0e-9;
    }
    m.hor_.mode = ( ui->checkBox->isChecked() ) ? 2 : 0;
    m.hor_.nbrAvgWaveforms = ui->spinBox->value();
}
