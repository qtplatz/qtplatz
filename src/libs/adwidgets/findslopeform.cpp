/**************************************************************************
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adportable/debug.hpp>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSignalBlocker>
#include <ratio>

using namespace adwidgets;

findSlopeForm::findSlopeForm(QWidget *parent) : QWidget(parent)
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

    // ThresholdAlgo
    ui->radioButton->setChecked( true );
    connect( ui->radioButton, &QRadioButton::toggled, [this] ( bool ) { emit valueChanged( channel_ ); } ); // Absolute
    connect( ui->radioButton_2, &QRadioButton::toggled, [this] ( bool ) { emit valueChanged( channel_ ); } ); // Average
    connect( ui->radioButton_3, &QRadioButton::toggled, [this] ( bool ) { emit valueChanged( channel_ ); } ); // Differential

    // Filter enable|disable
    connect( ui->groupBox_filter, &QGroupBox::toggled, [this] ( bool on ) { emit valueChanged( channel_ ); } );
    connect( ui->radioButton_dft, &QRadioButton::toggled, [this] ( bool on ) { emit valueChanged( channel_ ); } ); // DFT

    connect( ui->spinBox_sg, static_cast<void( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), [this] ( int ) {
            emit valueChanged( channel_ ); } ); // SG

    connect( ui->spinBox_dft_high, static_cast<void( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), [this] ( int ) {
            emit valueChanged( channel_ ); } ); // DFT (low pass)

    connect( ui->spinBox_dft_low, static_cast<void( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), [this] ( int ) {
            emit valueChanged( channel_ ); } ); // DFT (high pass)

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
    QSignalBlocker block_this( this );

    const QSignalBlocker bloks [] = {
        QSignalBlocker( ui->groupBox ), QSignalBlocker( ui->groupBox_filter )
        , QSignalBlocker( ui->doubleSpinBox ), QSignalBlocker( ui->doubleSpinBox_resolution ), QSignalBlocker( ui->doubleSpinBox_resp )
        , QSignalBlocker( ui->spinBox_sg ), QSignalBlocker( ui->spinBox_dft_high ), QSignalBlocker( ui->spinBox_dft_low )
        , QSignalBlocker( ui->radioButton_pos ), QSignalBlocker( ui->radioButton_neg ), QSignalBlocker( ui->radioButton_sg ), QSignalBlocker( ui->radioButton_dft )
    };
    (void)bloks;

    QSignalBlocker block( this );

    ui->groupBox->setChecked( m.enable );
    ui->doubleSpinBox->setValue( m.threshold_level * 1.0e3 );           // --> mV
    ui->doubleSpinBox_resolution->setValue( m.time_resolution * 1.0e9 ); // --> ns
    ui->doubleSpinBox_resp->setValue( m.response_time * 1.0e9 );         // --> ns

    // Slope
    ui->radioButton_neg->setChecked( m.slope == adcontrols::threshold_method::CrossDown ); // NEG
    ui->radioButton_pos->setChecked( m.slope == adcontrols::threshold_method::CrossUp );   // POS

    // ThresholdAlgo
    if ( m.algo_ == adcontrols::threshold_method::Absolute )
        ui->radioButton->setChecked( true );
    else if ( m.algo_ == adcontrols::threshold_method::AverageRelative )
        ui->radioButton_3->setChecked( true );
    else if ( m.algo_ == adcontrols::threshold_method::Differential )
        ui->radioButton_2->setChecked( true );

    // Filter
    ui->groupBox_filter->setChecked( m.use_filter );
    switch( m.filter ) {
    case adcontrols::threshold_method::DFT_Filter:   ui->radioButton_dft->setChecked( true ); break;
    case adcontrols::threshold_method::SG_Filter:    ui->radioButton_sg->setChecked( true );   break;
    }
    ui->spinBox_sg->setValue( m.sgwidth * 1.0e9 );    // s --> ns
    ui->spinBox_dft_high->setValue( m.hCutoffHz * 1.0e-6 ); // Hz --> MHz
    ui->spinBox_dft_low->setValue( m.lCutoffHz * 1.0e-6 ); // Hz --> MHz
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

    if ( ui->radioButton->isChecked() )
        m.algo_ = adcontrols::threshold_method::Absolute;
    else if ( ui->radioButton_3->isChecked() )
        m.algo_ = adcontrols::threshold_method::AverageRelative;
    else if ( ui->radioButton_2->isChecked() )
        m.algo_ = adcontrols::threshold_method::Differential;

    m.use_filter = ui->groupBox_filter->isChecked();
    if ( ui->radioButton_sg->isChecked() )
        m.filter = adcontrols::threshold_method::SG_Filter;
    else
        m.filter = adcontrols::threshold_method::DFT_Filter;
    m.sgwidth = ui->spinBox_sg->value() / std::nano::den;        // * 1.0e-9;// ns -> s
    m.hCutoffHz = ui->spinBox_dft_high->value() * std::mega::num;//1.0e6;    // MHz -> Hz
    m.lCutoffHz = ui->spinBox_dft_low->value() * std::mega::num; //1.0e6;    // MHz -> Hz
    m.complex_ = ui->checkBox->isChecked();
}

int
findSlopeForm::channel() const
{
    return channel_;
}

void
findSlopeForm::setJson( const QByteArray& json )
{
    ADDEBUG() << "#### " << json.toStdString();

    auto doc = QJsonDocument::fromJson( json );
    const auto& jobj = doc.object();
    const auto& obj = jobj[ "threshold_method" ];

    ui->groupBox->setChecked( obj[ "enable" ].toBool() );    //ui->groupBox->setChecked( m.enable );
    ui->doubleSpinBox->setValue( obj[ "threshold_level" ].toDouble() * 1.0e3 );           // ui->doubleSpinBox->setValue( m.threshold_level * 1.0e3 );           // --> mV
    ui->doubleSpinBox_resolution->setValue( obj[ "time_resolution"].toDouble() * 1.0e9 ); // --> ns
    ui->doubleSpinBox_resp->setValue( obj[ "response_time" ].toDouble() * 1.0e9 );         // --> ns

    // Slope
    ui->radioButton_neg->setChecked( obj[ "slope" ].toString() == "CrossDown" ); // NEG
    ui->radioButton_pos->setChecked( obj[ "slope" ].toString() == "CrossUp" );   // POS

    // ThresholdAlgo
    if ( obj[ "algo" ].toString() == "Absolute" )
        ui->radioButton->setChecked( true );
    else if ( obj[ "algo" ].toString() == "AverageRelative" )
        ui->radioButton_3->setChecked( true );
    else if ( obj[ "algo" ].toString() == "Differential" )
        ui->radioButton_2->setChecked( true );

    // Filter
    ui->groupBox_filter->setChecked( obj[ "use_filter" ].toBool() );
    if ( obj[ "filter" ].toString() == "DFT_Filter" )
        ui->radioButton_dft->setChecked( true );
    if ( obj[ "filter" ].toString() == "SG_Filter" )
        ui->radioButton_sg->setChecked( true );

    ui->spinBox_sg->setValue( obj[ "sgwidth" ].toDouble() * 1.0e9 );    // s --> ns
    ui->spinBox_dft_high->setValue( obj [ "hCutoffHz" ].toDouble() * 1.0e-6 ); // Hz --> MHz
    ui->spinBox_dft_low->setValue( obj [ "lCutoffHz" ].toDouble() * 1.0e-6 ); // Hz --> MHz
    ui->checkBox->setChecked( obj ["complex" ].toBool() );
}

QByteArray
findSlopeForm::readJson() const
{
    QString algo;
    if ( ui->radioButton->isChecked() )
        algo = "Absolute";
    else if ( ui->radioButton_3->isChecked() )
        algo = "AverageRelative";
    else if ( ui->radioButton_2->isChecked() )
        algo = "Differential";

    QJsonObject obj {
        { "title",             ui->groupBox->title() }
        , { "enable",          ui->groupBox->isChecked()                                   } // m.enable
        , { "threshold_level", ui->doubleSpinBox->value() * 1.0e-3                         } // mV -> V
        , { "time_resolution", ui->doubleSpinBox_resolution->value() * 1.0e-9              } // ns -> seconds
        , { "slope",           ui->radioButton_neg->isChecked() ? "CrossDown" : "CrossUp"  }
        , { "algo",            algo                                                        }
        , { "use_filter",      ui->groupBox_filter->isChecked()                            }
        , { "filter",          ui->radioButton_sg->isChecked() ? "SG_Filter" : "DFT_Filter"}
        , { "sgwidth",         double( ui->spinBox_sg->value() / std::nano::den )          } // * 1.0e-9;// ns -> s
        , { "hCutoffHz",       double( ui->spinBox_dft_high->value() * std::mega::num )    } //1.0e6;    // MHz -> Hz
        , { "lCutoffHz",       double( ui->spinBox_dft_low->value() * std::mega::num )     } //1.0e6;    // MHz -> Hz
        , { "complex",         ui->checkBox->isChecked() }
    };

    QJsonObject jobj;
    jobj [ "threshold_method" ] = obj;

    QJsonDocument jdoc( jobj );

    return QByteArray( jdoc.toJson( /* QJsonDocument::Indented */ ) );
}

// static
QByteArray
findSlopeForm::toJson( const adcontrols::threshold_method& m )
{
    QString algo;
    if ( m.algo_ == adcontrols::threshold_method::Absolute )
        algo = "Absolute";
    else if ( m.algo_ == adcontrols::threshold_method::AverageRelative )
        algo = "AverageRelative";
    else if ( m.algo_ == adcontrols::threshold_method::Differential )
        algo = "Differential";
    else if ( m.algo_ == adcontrols::threshold_method::AcqirisPKD )
        algo = "AcqirisPKD";
    else
        algo = QString::number( m.algo_ );

    QJsonObject obj {
        { "title",             ""                                       }
        , { "enable",          m.enable                                 }
        , { "threshold_level", m.threshold_level                        }
        , { "time_resolution", m.time_resolution                        }
        , { "slope",           m.slope == adcontrols::threshold_method::CrossDown ? "CrossDown" : "CrossUp"  }
        , { "algo",            algo                                     }
        , { "use_filter",      m.use_filter                             }
        , { "filter",          m.filter == adcontrols::threshold_method::SG_Filter ? "SG_Filter" : "DFT_Filter" }
        , { "sgwidth",         m.sgwidth                                }
        , { "hCutoffHz",       m.hCutoffHz                              }
        , { "lCutoffHz",       m.lCutoffHz                              }
        , { "complex",         m.complex_                               }
    };

    QJsonObject jobj;
    jobj [ "threshold_method" ] = obj;

    QJsonDocument jdoc( jobj );

    return QByteArray( jdoc.toJson( QJsonDocument::Compact ) );
}

// static
bool
findSlopeForm::fromJson( const QByteArray& json, adcontrols::threshold_method& m )
{
    auto doc = QJsonDocument::fromJson( json );
    const auto& jobj = doc.object();
    const auto& obj = jobj[ "threshold_method" ];

    m.enable = obj[ "enable" ].toBool();
    m.threshold_level = obj[ "threshold_level" ].toDouble();
    m.time_resolution = obj[ "time_resolution" ].toDouble();
    m.response_time   = obj[ "response_time" ].toDouble();
    m.slope = obj[ "slope" ].toString() == "CrossDown" ? adcontrols::threshold_method::CrossDown : adcontrols::threshold_method::CrossUp;

    // ThresholdAlgo
    if ( obj[ "algo" ].toString() == "Absolute" )
        m.algo_ = adcontrols::threshold_method::Absolute;
    else if ( obj[ "algo" ].toString() == "AverageRelative" )
        m.algo_ = adcontrols::threshold_method::AverageRelative;
    else if ( obj[ "algo" ].toString() == "Differential" )
        m.algo_ = adcontrols::threshold_method::Differential;
    else if ( obj[ "algo" ].toString() == "AcqirisPKD" )
        m.algo_ = adcontrols::threshold_method::AcqirisPKD;
    else
        m.algo_ = static_cast< adcontrols::threshold_method::ThresholdAlgo >( obj[ "algo" ].toInt() );

    m.use_filter = obj[ "use_filter" ].toBool();
    m.filter = obj[ "filter" ].toString() == "SG_Filter" ? adcontrols::threshold_method::SG_Filter : adcontrols::threshold_method::DFT_Filter;

    m.sgwidth = obj[ "sgwidth" ].toDouble();
    m.hCutoffHz = obj[ "hCutoffHz" ].toDouble();
    m.lCutoffHz = obj[ "lCutoffHz" ].toDouble();
    m.complex_ = obj[ "complex" ].toBool();

    return true;
}
