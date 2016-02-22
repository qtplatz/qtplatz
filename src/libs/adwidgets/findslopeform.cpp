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
#include <adcontrols/threshold_method.hpp>
#include <QSignalBlocker>

using namespace adwidgets;

findSlopeForm::findSlopeForm(QWidget *parent) :  QWidget(parent)
                                              , ui(new Ui::findSlopeForm)
                                              , channel_(0)
    
{
    ui->setupUi(this);

    // CH-[1|2] Group box
    connect( ui->groupBox, &QGroupBox::toggled, [this] ( bool on ) { emit valueChanged( channel_ ); });

    // Threshold (mV)
    connect( ui->doubleSpinBox, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged )
             , [this] ( double value ) {
        emit valueChanged( channel_ ); } );

    ui->doubleSpinBox->setSingleStep( 0.001 ); // 1uV step
    ui->doubleSpinBox->setAccelerated( true );

    // Time resolution (ns)
    connect( ui->doubleSpinBox_resolution, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged )
             , [this] ( double value ) { emit valueChanged( channel_ ); } );
    ui->doubleSpinBox_resolution->setMinimum( 0 );
    ui->doubleSpinBox_resolution->setMaximum( 1000 ); // 1us
    ui->doubleSpinBox_resolution->setSingleStep( 0.01 );  // 10ps step
    
    // Response time (ns)
    ui->doubleSpinBox_resp->setMinimum( 0 );    
    ui->doubleSpinBox_resp->setMaximum( 100000 );
    ui->doubleSpinBox_resp->setSingleStep( 0.01 );  // 10ps step
    connect( ui->doubleSpinBox_resp, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged )
             , [this] ( double value ) { emit valueChanged( channel_ ); } );

    // Slope
    connect( ui->radioButton_pos, &QRadioButton::toggled, [this] ( bool ) { emit valueChanged( channel_ ); } ); // POS
    connect( ui->radioButton_neg, &QRadioButton::toggled, [this] ( bool ) { emit valueChanged( channel_ ); } ); // NEG

    // Filter enable|disable
    connect( ui->groupBox_filter, &QGroupBox::toggled, [this] ( bool on ) { emit valueChanged( channel_ ); } );
    connect( ui->radioButton_dft, &QRadioButton::toggled, [this] ( bool on ) { emit valueChanged( channel_ ); } ); // DFT
    
    connect( ui->spinBox_sg, static_cast<void( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), [this] ( int ) { 
            emit valueChanged( channel_ ); } ); // SG

    connect( ui->spinBox_dft, static_cast<void( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), [this] ( int ) {
            emit valueChanged( channel_ ); } ); // DFT

    // complex or real
    connect( ui->checkBox, &QCheckBox::toggled, [this] ( bool ) {
            emit valueChanged( channel_ ); } );
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
findSlopeForm::set( const adcontrols::threshold_method& m )
{
    const QSignalBlocker bloks [] = {
        QSignalBlocker( ui->groupBox ), QSignalBlocker( ui->groupBox_filter )
        , QSignalBlocker( ui->doubleSpinBox ), QSignalBlocker( ui->doubleSpinBox_resolution ), QSignalBlocker( ui->doubleSpinBox_resp )
        , QSignalBlocker( ui->spinBox_sg ), QSignalBlocker( ui->spinBox_dft )
        , QSignalBlocker( ui->radioButton_pos ), QSignalBlocker( ui->radioButton_neg ), QSignalBlocker( ui->radioButton_sg ), QSignalBlocker( ui->radioButton_dft )
    };

    QSignalBlocker block( this );

    ui->groupBox->setChecked( m.enable );
    ui->doubleSpinBox->setValue( m.threshold_level * 1.0e3 );           // --> mV
    ui->doubleSpinBox_resolution->setValue( m.time_resolution * 1.0e9 ); // --> ns
    ui->doubleSpinBox_resp->setValue( m.response_time * 1.0e9 );         // --> ns

    // Slope
    ui->radioButton_neg->setChecked( m.slope == adcontrols::threshold_method::CrossDown ); // NEG
    ui->radioButton_pos->setChecked( m.slope == adcontrols::threshold_method::CrossUp );   // POS

    // Filter
    ui->groupBox_filter->setChecked( m.use_filter );
    switch( m.filter ) {
    case adcontrols::threshold_method::DFT_Filter:   ui->radioButton_dft->setChecked( true ); break;
    case adcontrols::threshold_method::SG_Filter:    ui->radioButton_sg->setChecked( true );   break;
    }
    ui->spinBox_sg->setValue( m.sgwidth * 1.0e9 );    // s --> ns
    ui->spinBox_dft->setValue( m.cutoffHz * 1.0e-6 ); // Hz --> MHz
    ui->checkBox->setChecked( m.complex_ );
}

void
findSlopeForm::get( adcontrols::threshold_method& m ) const
{
    m.enable = ui->groupBox->isChecked();
    m.threshold_level = ui->doubleSpinBox->value() * 1.0e-3; // mV -> V
    m.time_resolution = ui->doubleSpinBox_resolution->value() * 1.0e-9; // ns -> seconds
    m.response_time = ui->doubleSpinBox_resp->value() * 1.0e-9; // ns -> seconds
    m.slope = ui->radioButton_neg->isChecked() ? adcontrols::threshold_method::CrossDown : adcontrols::threshold_method::CrossUp;
    m.use_filter = ui->groupBox_filter->isChecked();
    if ( ui->radioButton_sg->isChecked() )
        m.filter = adcontrols::threshold_method::SG_Filter;
    else 
        m.filter = adcontrols::threshold_method::DFT_Filter;
    m.sgwidth = ui->spinBox_sg->value() * 1.0e-9;           // ns -> s
    m.cutoffHz = ui->spinBox_dft->value() * 1.0e6;          // MHz -> Hz
    m.complex_ = ui->checkBox->isChecked();
}

int
findSlopeForm::channel() const
{
    return channel_;
}

