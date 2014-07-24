/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mslockform.hpp"
#include "ui_mslockform.h"
#include <adcontrols/mslockmethod.hpp>
#include <adcontrols/processmethod.hpp>

using namespace adwidgets;

MSLockForm::MSLockForm(QWidget *parent) : QWidget(parent), ui(new Ui::MSLockForm)
{
    ui->setupUi(this);
}

MSLockForm::~MSLockForm()
{
    delete ui;
}

void
MSLockForm::setTitle( const QString& title )
{
    ui->groupBox->setTitle( title );
}

void
MSLockForm::setChecked( bool v )
{
    ui->groupBox->setChecked( v );
}

bool
MSLockForm::isChecked()
{
    return ui->groupBox->isChecked();
}

bool
MSLockForm::setContents( const adcontrols::MSLockMethod& m )
{
    setChecked( m.enabled() );

    if ( m.toleranceMethod() == adcontrols::MSLockMethod::idToleranceMethodDa ) {
        ui->radioButtonDa->setChecked( true );
    }
    else {
        ui->radioButtonPpm->setChecked( true );
    }
    ui->doubleSpinBoxDa->setValue( m.tolerance( adcontrols::MSLockMethod::idToleranceMethodDa ) );
    ui->doubleSpinBoxPpm->setValue( m.tolerance( adcontrols::MSLockMethod::idToleranceMethodPpm ) );

    if ( m.algorithm() == adcontrols::MSLockMethod::idMostAbundantPeak )
        ui->radioButtonAlgo0->setChecked( true );
    else
        ui->radioButtonAlgo1->setChecked( true );

    ui->checkBoxThreshold->setChecked( m.enablePeakThreshold() );
    ui->doubleSpinBoxThreshold->setValue( m.peakIntensityThreshold() );

    return true;
}

bool
MSLockForm::getContents( adcontrols::MSLockMethod& m )
{
    m.setEnabled( ui->groupBox->isChecked() );
    if ( ui->radioButtonDa->isChecked() )
        m.setToleranceMethod( adcontrols::MSLockMethod::idToleranceMethodDa );
    else
        m.setToleranceMethod( adcontrols::MSLockMethod::idToleranceMethodPpm );

    m.setTolerance( adcontrols::MSLockMethod::idToleranceMethodDa, ui->doubleSpinBoxDa->value() );
    m.setTolerance( adcontrols::MSLockMethod::idToleranceMethodPpm, ui->doubleSpinBoxPpm->value() );

    m.setAlgorithm( ui->radioButtonAlgo0->isChecked() ? adcontrols::MSLockMethod::idMostAbundantPeak : adcontrols::MSLockMethod::idClosestPeak );
    m.setPeakIntensityThreshold( ui->doubleSpinBoxThreshold->value() );
    m.setEnablePeakThreshold( ui->checkBoxThreshold->isChecked() );

    return true;
}

bool
MSLockForm::setContents( const adcontrols::ProcessMethod& pm, bool bForceCreate )
{
    if ( auto ptgt = pm.find< adcontrols::MSLockMethod >() ) {
        setContents( *ptgt );
        return true;
    }
    if ( bForceCreate ) {
        setContents( adcontrols::MSLockMethod() );
        return true;
    }
        
    return false;
}

bool
MSLockForm::getContents( adcontrols::ProcessMethod& pm )
{
    adcontrols::MSLockMethod lkm;
    getContents( lkm );
    pm.appendMethod( lkm );
    return true;
}
