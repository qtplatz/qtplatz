/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "thresholdactionform.hpp"
#include "ui_thresholdactionform.h"
#include <adcontrols/threshold_action.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/scanlaw.hpp>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QSignalBlocker>

namespace adwidgets {
}

using namespace adwidgets;

ThresholdActionForm::ThresholdActionForm(QWidget *parent) : QWidget(parent)
                                                          , ui( new Ui::ThresholdActionForm )
{
    ui->setupUi(this);
    connect( ui->groupBox, &QGroupBox::toggled, [this] (bool) { emit valueChanged(); } );
    connect( ui->checkBox, &QCheckBox::toggled, [this] (bool) { emit valueChanged(); } );
    connect( ui->checkBox_2, &QCheckBox::toggled, [this] (bool) { emit valueChanged(); } );
    connect( ui->checkBox_3, &QCheckBox::toggled, [this] (bool) { emit valueChanged(); } );
    connect( ui->doubleSpinBox, static_cast< void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this] (double) { emit valueChanged(); } );
    connect( ui->doubleSpinBox_2, static_cast< void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this] (double) { emit valueChanged(); } );
    connect( ui->lineEdit, &QLineEdit::editingFinished, [this](){ formulaChanged( ui->lineEdit->text() ); } );
    connect( ui->spinBox, static_cast< void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this] (int mode) { modeChanged( mode ); } );
    connect( ui->doubleSpinBox_3, static_cast< void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this] (double mass) { massChanged( mass ); } );
}

ThresholdActionForm::~ThresholdActionForm()
{
    delete ui;
}

void
ThresholdActionForm::OnCreate( const adportable::Configuration& )
{
}

void
ThresholdActionForm::OnInitialUpdate()
{
}

void
ThresholdActionForm::OnFinalClose()
{
}

void
ThresholdActionForm::onUpdate( boost::any& )
{
}

bool
ThresholdActionForm::getContents( boost::any& ) const
{
    Q_ASSERT( 0 );
    return false;
}

bool
ThresholdActionForm::setContents( boost::any&& )
{
    Q_ASSERT( 0 );    
    return false;
}

bool
ThresholdActionForm::get( adcontrols::threshold_action& m ) const
{
    m.enable = ui->groupBox->isChecked();
    m.recordOnFile = ui->checkBox->isChecked();
    m.enableTimeRange = ui->checkBox_3->isChecked();
    m.exclusiveDisplay = ui->checkBox_2->isChecked();
    m.delay = ui->doubleSpinBox->value() * 1.0e-6;
    m.width = ui->doubleSpinBox_2->value() * 1.0e-6;

    if ( auto sp = spectrometer_.lock() ) {
        m.objid_spectrometer = sp->objtext();
        m.formula = ui->lineEdit->text().toStdString();
        m.mode = ui->spinBox->value();
        m.mass = ui->doubleSpinBox_3->value();
    }

    return true;
}

bool
ThresholdActionForm::set( const adcontrols::threshold_action& m )
{
    QSignalBlocker block_this( this );
#if 0
    QSignalBlocker blocks[] = { QSignalBlocker( ui->groupBox ), QSignalBlocker( ui->checkBox ), QSignalBlocker( ui->checkBox_2 )
        , QSignalBlocker( ui->checkBox_3 ), QSignalBlocker( ui->doubleSpinBox ), QSignalBlocker( ui->doubleSpinBox_2 )
    , QSignalBlocker( ui->lineEdit ), QSignalBlocker( ui->doubleSpinBox_3 ), QSignalBlocker( ui->spinBox ) };
#endif

    ui->checkBox->setChecked( m.enable );
    ui->checkBox_2->setChecked( m.exclusiveDisplay );
    ui->checkBox_3->setChecked( m.recordOnFile );
    ui->doubleSpinBox->setValue( m.delay * 1.0e6 );
    ui->doubleSpinBox_2->setValue( m.width * 1.0e6 );
    ui->lineEdit->setText( QString::fromStdString( m.formula ) );
    ui->spinBox->setValue( m.mode );
    ui->doubleSpinBox_3->setValue( m.mass );

    return true;
}

void
ThresholdActionForm::setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > sp )
{
    spectrometer_ = sp;
    if ( sp ) {

        ui->groupBox_2->setEnabled( true );
        QString text = QString("[%1]").arg( QString::fromStdString( sp->objtext() ));
        ui->groupBox_2->setTitle( text );

    } else {

        ui->groupBox_2->setEnabled( false );        

    }
}

std::shared_ptr< const adcontrols::MassSpectrometer >
ThresholdActionForm::massSpectrometer() const
{
    auto spectrometer = spectrometer_.lock();
    return spectrometer;
}

void
ThresholdActionForm::formulaChanged( const QString& formula )
{
    if ( auto sp = spectrometer_.lock() ) {

        QSignalBlocker blocks[] = { QSignalBlocker( ui->doubleSpinBox_3 ), QSignalBlocker( ui->doubleSpinBox ) };
        
        double mass = adcontrols::ChemicalFormula().getMonoIsotopicMass( formula.toStdString() );
        ui->doubleSpinBox_3->setValue( mass );

        auto mode = ui->spinBox->value();

        double time = sp->getScanLaw().getTime( mass, mode );
        ui->doubleSpinBox->setValue( adcontrols::metric::scale_to_micro( time ) );            
        
        emit valueChanged();
    }
}

void
ThresholdActionForm::modeChanged( int mode )
{
    if ( auto sp = spectrometer_.lock() ) {

        double mass = ui->doubleSpinBox_3->value();
        double time = sp->getScanLaw().getTime( mass, mode );
        
        QSignalBlocker block( ui->doubleSpinBox );
        ui->doubleSpinBox->setValue( adcontrols::metric::scale_to_micro( time ) );            
        
        emit valueChanged();
    }
}

void
ThresholdActionForm::massChanged( double mass )
{
    if ( auto sp = spectrometer_.lock() ) {

        double time = sp->getScanLaw().getTime( mass, ui->spinBox->value() );        

        QSignalBlocker block( ui->doubleSpinBox );
        
        ui->doubleSpinBox->setValue( adcontrols::metric::scale_to_micro( time ) );            
        
        emit valueChanged();
    }
}
