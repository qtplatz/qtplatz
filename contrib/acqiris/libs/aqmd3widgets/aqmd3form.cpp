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

#include "aqmd3form.hpp"
#include "ui_aqmd3form.h"
#include "constants.hpp"
#include <adportable/debug.hpp>
#include <aqmd3controls/method.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <QSignalBlocker>

using namespace aqmd3widgets;

aqmd3Form::aqmd3Form( QWidget *parent ) : QWidget( parent )
                                          , ui( new Ui::aqmd3Form )
                                          , sampRate_( 3.2e9 )
{
    ui->setupUi( this );

    connect( ui->pushButton, &QPushButton::pressed, [=](){  emit valueChanged( idAQMD3Any, 0, QVariant() ); } );

    ui->spinBox->setStepBy( []( adwidgets::SpinBox * _this, int step ) {
            int index = _this->value() >= 8 ? ( _this->value() ) / 8 + 7 : _this->value();
            index += step;
            int value = index <= 8 ? index : ( index - 7 ) * 8;
            _this->setValue( value );
        });

    // start delay
    connect( ui->doubleSpinBox_1, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), [this] ( double d ) {
            ui->doubleSpinBox_1->setStyleSheet( "QDoubleSpinBox { color: #ff6347; }" );
            emit valueChanged( idAQMD3StartDelay, 0, QVariant( adcontrols::metric::scale_to_base( d, adcontrols::metric::micro ) ) );
        } );

    // width
    connect( ui->doubleSpinBox_2, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), [this] ( double d ) {
            ui->doubleSpinBox_2->setStyleSheet( "QDoubleSpinBox { color: #ff6347; }" );
            emit valueChanged( idAQMD3Width, 0, QVariant( adcontrols::metric::scale_to_base( d, adcontrols::metric::micro ) ) );
        });

    // external trig delay
    connect( ui->doubleSpinBox, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), [this] ( double d ) {
            ui->doubleSpinBox->setStyleSheet( "QDoubleSpinBox { color: #ff6347; }" );
            emit valueChanged( idAQMD3ExtDelay, 0, QVariant( adcontrols::metric::scale_to_base( d, adcontrols::metric::micro ) ) );
        });

    // number of average
    connect( ui->spinBox, static_cast<void( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), [this] ( int d ) {
            if ( d > 8 && ( d % 8 ) ) {
                QSignalBlocker block( ui->spinBox );
                ui->spinBox->setValue( d & ~07 );
            }
            ui->spinBox->setStyleSheet( "QSpinBox { color: #ff6347; }" );
            emit valueChanged( idNbrAverages, 0, QVariant( d ) );
        } );

    // MultiRecord
    connect( ui->spinBox_2, static_cast<void( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), [this] ( int d ) {
            ui->spinBox_2->setStyleSheet( "QSpinBox { color: #ff6347; }" );
            // emit valueChanged( idNbrRecords, 0, QVariant( d ) );
        } );

    // mode
    connect( ui->checkBox_Avg, &QCheckBox::toggled, [this] ( bool flag ) {
            ui->checkBox_Avg->setStyleSheet( "QCheckBox { color: #ff6347; }" );
            if ( flag && ui->checkBox->isChecked() ) // exclusive to TSR
                ui->checkBox->setChecked( false );
            if ( !flag && ui->checkBox_pkd->isChecked() )
                ui->checkBox_pkd->setChecked( false );
            emit valueChanged( idAQMD3Mode, 0, QVariant( flag ) ); // Digitizer | Averager ==> disable ion counting
        } );

    // TSR
    connect( ui->checkBox, &QCheckBox::toggled, [this] ( bool flag ) {
            ui->checkBox->setStyleSheet( "QCheckBox { color: #ff6347; }" );
            if ( flag && ui->checkBox_Avg->isChecked() )
                ui->checkBox_Avg->setChecked( false );
            // emit valueChanged( idTSREnable, 0, QVariant( flag ) ); }
        });

    // PKD
    connect( ui->checkBox_pkd, &QCheckBox::toggled, [this] ( bool flag ) {
            ui->checkBox_pkd->setStyleSheet( "QCheckBox { color: #ff6347; }" );
            if ( flag && !ui->checkBox_Avg->isChecked() )
                ui->checkBox_Avg->setChecked( true );
            emit valueChanged( idPKDEnable, 0, QVariant( flag ) );
        } );
}

aqmd3Form::~aqmd3Form()
{
    delete ui;
}

void
aqmd3Form::onInitialUpdate()
{
}

void
aqmd3Form::setContents( const aqmd3controls::method& m )
{
    QSignalBlocker blocks[] = {
        QSignalBlocker( ui->doubleSpinBox_1 ), QSignalBlocker( ui->doubleSpinBox_2 ), QSignalBlocker( ui->doubleSpinBox )
        , QSignalBlocker( ui->spinBox ), QSignalBlocker( ui->checkBox_Avg ), QSignalBlocker( ui->spinBox_2 )
        , QSignalBlocker( ui->checkBox ) };

    ui->doubleSpinBox_1->setValue( adcontrols::metric::scale_to_micro( m.device_method().delay_to_first_sample_ ) );

    sampRate_ = m.device_method().samp_rate;
    double width = m.device_method().nbr_of_s_to_acquire_ / sampRate_;
    ui->doubleSpinBox_2->setValue( adcontrols::metric::scale_to_micro( width ) );

    if ( !m.protocols().empty() && !m.protocols() [ 0 ].delay_pulses().empty() ) {
        auto ext_trig_delay = m.protocols() [ 0 ].delay_pulses() [ adcontrols::TofProtocol::EXT_ADC_TRIG ].first;
        ui->doubleSpinBox->setValue( adcontrols::metric::scale_to_micro( ext_trig_delay ) );
    }

    ui->spinBox->setValue( m.device_method().nbr_of_averages );

    ui->checkBox_Avg->setChecked( !( m.mode() == aqmd3controls::method::DigiMode::Digitizer ) );
    ui->checkBox_pkd->setChecked( m.device_method().pkd_enabled );

    ui->spinBox_2->setValue( m.device_method().nbr_records );

    ui->checkBox->setChecked( m.device_method().TSR_enabled );

    for ( auto w : { ui->doubleSpinBox_1, ui->doubleSpinBox_2 } )
        w->setStyleSheet( "QDoubleSpinBox { color: black; }" );
    ui->spinBox->setStyleSheet( "QSpinBox { color: black; }" );
    ui->spinBox_2->setStyleSheet( "QSpinBox { color: black; }" );
    ui->checkBox->setStyleSheet( "QCheckBox { color: black; }" );
    ui->checkBox_Avg->setStyleSheet( "QCheckBox { color: black; }" );
    ui->checkBox_pkd->setStyleSheet( "QCheckBox { color: black; }" );
}

void
aqmd3Form::getContents( aqmd3controls::method& m )
{
    m.device_method().delay_to_first_sample_ = adcontrols::metric::scale_to_base( ui->doubleSpinBox_1->value(), adcontrols::metric::micro );

    double width = adcontrols::metric::scale_to_base( ui->doubleSpinBox_2->value(), adcontrols::metric::micro );
    m.device_method().nbr_of_s_to_acquire_ = uint32_t( width * m.device_method().samp_rate + 0.5 );

    auto ext_trig_delay = adcontrols::metric::scale_to_base( ui->doubleSpinBox->value(), adcontrols::metric::micro );
    if ( !m.protocols().empty() && !m.protocols() [ 0 ].delay_pulses().empty() ) {
        m.protocols() [ 0 ].delay_pulses() [ adcontrols::TofProtocol::EXT_ADC_TRIG ] = std::make_pair( ext_trig_delay, 1.0e-6 );
    }

    m.device_method().nbr_of_averages = ui->spinBox->value();

    m.setMode( ui->checkBox_Avg->isChecked() ? aqmd3controls::method::DigiMode::Averager : aqmd3controls::method::DigiMode::Digitizer );

    m.device_method().nbr_records = ui->spinBox_2->value();

    m.device_method().TSR_enabled = ui->checkBox->isChecked();

    m.device_method().pkd_enabled = ui->checkBox_pkd->isChecked();

    for ( auto w : { ui->doubleSpinBox_1, ui->doubleSpinBox_2 } )
        w->setStyleSheet( "QDoubleSpinBox { color: black; }" );

    ui->spinBox->setStyleSheet( "QSpinBox { color: black; }" );
    ui->spinBox_2->setStyleSheet( "QSpinBox { color: black; }" );
    ui->checkBox->setStyleSheet( "QCheckBox { color: black; }" );
    ui->checkBox_Avg->setStyleSheet( "QCheckBox { color: black; }" );
    ui->checkBox_pkd->setStyleSheet( "QCheckBox { color: black; }" );
}

void
aqmd3Form::onHandleValue( idCategory id, int channel, const QVariant& value )
{
    switch ( id ) {
    case idGlobalAny:
    case idSlopeTimeConverter:
    case idHorizontal:
    case idVertical:
    case idChannels:
    case idTrigger:
    case idAQMD3Any:
    case idAQMD3Mode:
    case idAQMD3Width:
        break;
    case idAQMD3StartDelay:
        do {
            QSignalBlocker block( ui->doubleSpinBox_1 );
            ui->doubleSpinBox_1->setValue( adcontrols::metric::scale_to_micro( value.toDouble() ) );
        } while ( 0 );
        break;

    case idAQMD3NbrSamples:
        do {
            QSignalBlocker block( ui->doubleSpinBox_2 );
            double width = value.toInt() / sampRate_;
            ui->doubleSpinBox_2->setValue( adcontrols::metric::scale_to_micro( width ) );
        } while ( 0 );
        break;

    case idAQMD3SampRate:
        sampRate_ = value.toDouble();
        break;

    case idNbrAverages:
        do {
            QSignalBlocker block( ui->spinBox );
            ui->spinBox->setValue( value.toInt() );
        } while ( 0 );
        break;
    case idThresholdAction:
    case idNbrRecords:
    case idTSREnable:
    case idAQMD3ExtDelay:
    case idPKDEnable:
    default:
        break;
    }
}

void
aqmd3Form::setEnabled( const QString& name, bool enable )
{
    if ( name == "StartDelay" )
        ui->doubleSpinBox_1->setEnabled( enable );

    if ( name == "Width" )
        ui->doubleSpinBox_2->setEnabled( enable );

    if ( name == "ExtTrigDelay" )
        ui->doubleSpinBox->setEnabled( enable );
}
