/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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
TargetingForm::getContents( adcontrols::TargetingMethod& m )
{
	m.resolving_power( ui->doubleSpinBoxRP->value() );
	m.peak_width( ui->doubleSpinBoxWidth->value() );
    m.is_use_resolving_power( ui->radioButtonRP->isChecked() );

    m.chargeState( ui->spinBoxChargeMin->value(), ui->spinBoxChargeMax->value() );

    m.isLowMassLimitEnabled( ui->cbxLowMass->checkState() == Qt::Checked );
    m.isHighMassLimitEnabled( ui->cbxHighMass->checkState() == Qt::Checked );

	m.lowMassLimit( ui->doubleSpinBoxLowMassLimit->value() );
	m.highMassLimit( ui->doubleSpinBoxHighMassLimit->value() );
}

void
TargetingForm::setContents( const adcontrols::TargetingMethod& m )
{
}
