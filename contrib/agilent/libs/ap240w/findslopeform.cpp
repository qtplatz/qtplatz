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
#include <ap240/digitizer.hpp>
#include <QSignalBlocker>

using namespace ap240w;

findSlopeForm::findSlopeForm(QWidget *parent) :  QWidget(parent)
                                              , ui(new Ui::findSlopeForm)
                                              , channel_(0)
    
{
    ui->setupUi(this);

    // CH-[1|2] Group box
    connect( ui->groupBox, &QGroupBox::toggled, [this] ( bool on ) { emit valueChanged( channel_ ); });

    // Threshold (mV)
    connect( ui->doubleSpinBox, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged )
             , [this] ( double value ) { emit valueChanged( channel_ ); } );

    // Time resolution (ns)
    connect( ui->doubleSpinBox_2, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged )
             , [this] ( double value ) { emit valueChanged( channel_ ); } );

    // Slope
    connect( ui->radioButton_3, &QRadioButton::toggled, [this] (bool) { emit valueChanged( channel_ ); } ); // NEG
    connect( ui->radioButton_4, &QRadioButton::toggled, [this] (bool) { emit valueChanged( channel_ ); } ); // POS

    // Filter enable|disable
    connect( ui->groupBox_2, &QGroupBox::toggled, [this] ( bool on ) { emit valueChanged( channel_ ); } );
    connect( ui->radioButton, &QRadioButton::toggled, [this] ( bool on ) { emit valueChanged( channel_ ); } ); // SG
    connect( ui->radioButton_2, &QRadioButton::toggled, [this] ( bool on ) { emit valueChanged( channel_ ); } ); // DFT
    connect( ui->radioButton_5, &QRadioButton::toggled, [this] ( bool on ) { emit valueChanged( channel_ ); } ); // IGN
    
    connect( ui->spinBox, static_cast<void( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), [this] ( int ) { emit valueChanged( channel_ ); } ); // IGN
    connect( ui->spinBox_2, static_cast<void( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), [this] ( int ) { emit valueChanged( channel_ ); } ); // SG
    connect( ui->spinBox_3, static_cast<void( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), [this] ( int ) { emit valueChanged( channel_ ); } ); // DFT
}

findSlopeForm::~findSlopeForm()
{
    delete ui;
}

void
findSlopeForm::setTitle( int ch, const QString& title )
{
    const QSignalBlocker blocker( this );

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
    const QSignalBlocker blocker( ui->groupBox );
    ui->groupBox->setChecked( on );
}

void
findSlopeForm::set( const ap240::threshold_method& m )
{
    const QSignalBlocker bloks [] = {
        QSignalBlocker( ui->groupBox ), QSignalBlocker( ui->groupBox_2 )
        , QSignalBlocker( ui->doubleSpinBox ), QSignalBlocker( ui->doubleSpinBox_2 )
        , QSignalBlocker( ui->spinBox ), QSignalBlocker( ui->spinBox_2 ), QSignalBlocker( ui->spinBox_3 )
        , QSignalBlocker( ui->radioButton ), QSignalBlocker( ui->radioButton_2 ), QSignalBlocker( ui->radioButton_3 ), QSignalBlocker( ui->radioButton_4 )
        , QSignalBlocker( ui->radioButton_5 )
    };


    ui->groupBox->setChecked( m.enable );
    ui->doubleSpinBox->setValue( m.threshold_level );
    ui->doubleSpinBox_2->setValue( m.time_resolution * 1.0e9 );    // --> ns

    // Slope
    ui->radioButton_3->setChecked( m.slope == ap240::threshold_method::CrossDown ); // NEG
    ui->radioButton_4->setChecked( m.slope == ap240::threshold_method::CrossUp );   // POS

    // Filter
    ui->groupBox->setChecked( m.use_filter );
    switch( m.filter ) {
    case ap240::threshold_method::DFT_Filter:   ui->radioButton_2->setChecked( true ); break;
    case ap240::threshold_method::IGN_Filter:   ui->radioButton_2->setChecked( true ); break;
    case ap240::threshold_method::SG_Filter:    ui->radioButton->setChecked( true );   break;
    }
    ui->spinBox->setValue( m.igPoints );
    ui->spinBox_2->setValue( m.sgPoints );
    ui->spinBox_3->setValue( m.cutOffMHz );
}

void
findSlopeForm::get( ap240::threshold_method& m ) const
{
    m.enable = ui->groupBox->isChecked();
    m.threshold_level = ui->doubleSpinBox->value();
    m.time_resolution = ui->doubleSpinBox_2->value() * 1.0e-9; // ns -> seconds
    m.slope = ui->radioButton_3->isChecked() ? ap240::threshold_method::CrossDown : ap240::threshold_method::CrossUp;
    m.use_filter = ui->groupBox_2->isChecked();
    if ( ui->radioButton->isChecked() )
        m.filter = ap240::threshold_method::SG_Filter;
    else if ( ui->radioButton_2->isChecked() )
        m.filter = ap240::threshold_method::DFT_Filter;
    else
        m.filter = ap240::threshold_method::IGN_Filter;
    m.igPoints = ui->spinBox->value();
    m.sgPoints = ui->spinBox_2->value();
    m.cutOffMHz = ui->spinBox_3->value();
}

int
findSlopeForm::channel() const
{
    return channel_;
}

