/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "targetingform.hpp"
#include "ui_targetingform.h"
#include "spin_t.hpp"
#include <adcontrols/targetingmethod.hpp>

using namespace adwidgets;

TargetingForm::TargetingForm(QWidget *parent) :  QWidget(parent)
                                              ,  ui(new Ui::TargetingForm)
{
    ui->setupUi(this);

    ui->radioButtonRP->setChecked( false );
    ui->radioButtonWidth->setChecked( true );    
    spin_t<QDoubleSpinBox, double>::init( ui->doubleSpinBoxRP, 1000.0, 100000.0, 10000.0 );
    spin_t<QDoubleSpinBox, double>::init( ui->doubleSpinBoxWidth, 0.1, 500.0, 1.0 );
    spin_t<QSpinBox, int >::init( ui->spinBoxChargeMin, 1, 50, 1 );
    spin_t<QSpinBox, int >::init( ui->spinBoxChargeMax, 1, 50, 3 );

	ui->cbxLowMass->setCheckState( Qt::Unchecked );
	ui->cbxHighMass->setCheckState( Qt::Unchecked );
	spin_t<QDoubleSpinBox, double >::init( ui->doubleSpinBoxLowMassLimit, 1, 5000,  100 );
	spin_t<QDoubleSpinBox, double >::init( ui->doubleSpinBoxHighMassLimit, 1, 5000, 2000 );
}

TargetingForm::~TargetingForm()
{
    delete ui;
}

void
TargetingForm::setTitle( const QString& title, bool enableCharge, bool enableLimits )
{
    ui->groupBox->setTitle( title );
    if ( !enableCharge ) {
        ui->cbxLowMass->setCheckState( Qt::Unchecked );
        ui->cbxHighMass->setCheckState( Qt::Unchecked );
        ui->cbxLowMass->setEnabled( false );
        ui->cbxHighMass->setEnabled( false );
    }
    if ( !enableLimits ) {
        ui->doubleSpinBoxLowMassLimit->setEnabled( false );
        ui->doubleSpinBoxHighMassLimit->setEnabled( false );
    }
}

void
TargetingForm::getContents( adcontrols::TargetingMethod& m )
{
    m.setTolerance( adcontrols::idTolerancePpm, ui->doubleSpinBoxRP->value() );

    m.setTolerance( adcontrols::idToleranceDaltons, ui->doubleSpinBoxWidth->value() / 1000.0 ); // mDa --> Da
    m.setToleranceMethod( ui->radioButtonRP->isChecked() ? adcontrols::idTolerancePpm : adcontrols::idToleranceDaltons );

    m.chargeState( ui->spinBoxChargeMin->value(), ui->spinBoxChargeMax->value() );

    m.isLowMassLimitEnabled( ui->cbxLowMass->checkState() == Qt::Checked );
    m.isHighMassLimitEnabled( ui->cbxHighMass->checkState() == Qt::Checked );

	m.lowMassLimit( ui->doubleSpinBoxLowMassLimit->value() );
	m.highMassLimit( ui->doubleSpinBoxHighMassLimit->value() );
}

void
TargetingForm::setContents( const adcontrols::TargetingMethod& m )
{
    ui->doubleSpinBoxRP->setValue( m.tolerance( adcontrols::idTolerancePpm ) );
    ui->doubleSpinBoxWidth->setValue( m.tolerance( adcontrols::idToleranceDaltons ) * 1000.0 );
    ui->radioButtonRP->setChecked( m.toleranceMethod() == adcontrols::idTolerancePpm );
    auto charge = m.chargeState();
    ui->spinBoxChargeMin->setValue( charge.first );
    ui->spinBoxChargeMax->setValue( charge.second );

    auto limits = m.isMassLimitsEnabled();

    ui->cbxLowMass->setCheckState( limits.first ? Qt::Checked : Qt::Unchecked );
    ui->cbxHighMass->setCheckState( limits.second ? Qt::Checked : Qt::Unchecked );

	ui->doubleSpinBoxLowMassLimit->setValue( m.lowMassLimit() );
	ui->doubleSpinBoxHighMassLimit->setValue( m.highMassLimit() );
}
