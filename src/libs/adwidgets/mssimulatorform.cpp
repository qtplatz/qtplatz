/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mssimulatormethod.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adportable/debug.hpp>
#include <infitofcontrols/constants.hpp> // clsid for massspectrometer
#include <QSignalBlocker>

using namespace adwidgets;

MSSimulatorForm::MSSimulatorForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MSSimulatorForm)
{
    ui->setupUi(this);

    ui->comboBox->setEnabled( false );
    ui->spinBox_lap->setEnabled( false );
    connect( ui->spinBox,     qOverload< int >( &QSpinBox::valueChanged ), [this]( int ){ emit onValueChanged(); } );
    connect( ui->spinBox_lap, qOverload< int >( &QSpinBox::valueChanged ), [this]( int ){ emit onValueChanged(); } );

    connect( ui->doubleSpinBox_3, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), [this] ( double ) { emit onValueChanged(); } );
    connect( ui->doubleSpinBox_4, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), [this] ( double ) { emit onValueChanged(); } );
    connect( ui->doubleSpinBox_5, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), [this] ( double ) { emit onValueChanged(); } );

    connect( ui->spinBox_2, qOverload< int >( &QSpinBox::valueChanged )
             , [this] ( int ) {
                   QSignalBlocker block( ui->spinBox_3 );
                   if ( ui->spinBox_3->value() < ui->spinBox_2->value() )
                       ui->spinBox_3->setValue( ui->spinBox_2->value() );
                   emit onValueChanged();
               } );

    connect( ui->spinBox_3, qOverload< int >( &QSpinBox::valueChanged )
             , [this] ( int ) {
                   QSignalBlocker block( ui->spinBox_2 );
                   if ( ui->spinBox_3->value() < ui->spinBox_2->value() )
                       ui->spinBox_2->setValue( ui->spinBox_3->value() );
                   emit onValueChanged();
               } );

    connect( ui->spinBox_3, qOverload< int >( &QSpinBox::valueChanged ), [this] ( int ) { emit onValueChanged(); } );

    //connect( ui->checkBox, &QCheckBox::toggled, [this](bool) { emit onValueChanged(); } );
    connect( ui->groupBox, &QGroupBox::toggled, [this](bool) { emit onValueChanged(); } );
    connect( ui->pushButton, &QPushButton::pressed, [this] () { emit triggerProcess(); } );

    connect( ui->comboBox, qOverload< int >( &QComboBox::currentIndexChanged ), this, [&](int index){
        ADDEBUG() << ui->comboBox->currentData().toInt();
    });
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
    m.setChargeStateMin( ui->spinBox_2->value() );
    m.setChargeStateMax( ui->spinBox_3->value() );
    m.setIsTof( ui->groupBox->isChecked() );
    m.setLength( ui->doubleSpinBox_3->value() );
    m.setAcceleratorVoltage( ui->doubleSpinBox_4->value() );
    m.setTDelay( ui->doubleSpinBox_5->value() * 1.0e-6 );
    m.setIsPositivePolarity( ui->radioButtonPos->isChecked() );

    m.setMode( ui->spinBox_lap->value() );
    m.setProtocol( ui->comboBox->currentIndex() );

    return true;
}

bool
MSSimulatorForm::setContents( const adcontrols::MSSimulatorMethod& m )
{
    QSignalBlocker blocks( this );

    ui->spinBox->setValue( m.resolvingPower() );

    ui->spinBox_2->setValue( m.chargeStateMin() );
    ui->spinBox_3->setValue( m.chargeStateMax() );
    ui->groupBox->setChecked ( m.isTof() );
    ui->doubleSpinBox_3->setValue( m.length() );
    ui->doubleSpinBox_4->setValue( m.acceleratorVoltage() );
    ui->doubleSpinBox_5->setValue( m.tDelay() * 1.0e6 );

    while ( ui->comboBox->count() <= m.protocol() )
        ui->comboBox->addItem( QString("p%1").arg( ui->comboBox->count() ) );

    ui->comboBox->setCurrentIndex( m.protocol() );
    ui->spinBox_lap->setValue( m.mode() );

    ui->radioButtonPos->setChecked( m.isPositivePolarity() );

    connect( ui->spinBox_lap, qOverload<int>(&QSpinBox::valueChanged), [&](int lap){
        if ( auto sp = massSpectrometer_.lock() ) {
            if ( auto law = sp->scanLaw() )
                ui->doubleSpinBox_3->setValue( law->fLength( lap ) );
        }
    });

    return true;
}

void
MSSimulatorForm::setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > p )
{
    QSignalBlocker block( this );
    massSpectrometer_ = p;
    if ( p ) {
        ui->doubleSpinBox_3->setValue( p->fLength() );
        ui->doubleSpinBox_4->setValue( p->acceleratorVoltage() );
        ui->doubleSpinBox_5->setValue( p->tDelay() * std::micro::den );
        if ( p->massSpectrometerClsid() == infitof::iids::uuid_massspectrometer ) {

            if ( auto law = p->scanLaw() )
                ui->doubleSpinBox_3->setValue( law->fLength( 0 ) );

            ui->comboBox->setEnabled( true );
            ui->spinBox_lap->setEnabled( true );
            ui->spinBox_lap->setValue( p->mode( 0 ) );
            ui->doubleSpinBox_4->setEnabled( false );
            ui->doubleSpinBox_5->setEnabled( false );
        } else {
            ui->comboBox->setEnabled( false );
            ui->spinBox_lap->setEnabled( false );
            ui->doubleSpinBox_4->setEnabled( true );
            ui->doubleSpinBox_5->setEnabled( true );
        }
    }
}

void
MSSimulatorForm::setMassSpectrum( std::shared_ptr< const adcontrols::MassSpectrum > p )
{
    massSpectrum_ = p;
    if ( p ) {
        ui->comboBox->clear();
        int proto(0);
        for ( const auto& fms: adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( *p ) )
            ui->comboBox->addItem( QString( "p%1" ).arg( proto++ ), fms.mode() );
        ui->spinBox_lap->setValue( p->mode() );
    }
}
