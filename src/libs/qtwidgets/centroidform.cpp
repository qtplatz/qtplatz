/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "centroidform.hpp"
#include "ui_centroidform.h"
#include <adcontrols/processmethod.hpp>
#include <adcontrols/centroidmethod.hpp>
#include <QStandardItemModel>
#include "centroiddelegate.hpp"
#include "standarditemhelper.hpp"
using namespace qtwidgets;

/////////////////////

CentroidForm::CentroidForm(QWidget *parent) : QWidget(parent)
                                            , ui(new Ui::CentroidForm)
                                            , pMethod_( new adcontrols::CentroidMethod ) 
{
	ui->setupUi(this);
}

CentroidForm::~CentroidForm()
{
    delete ui;
}

void
CentroidForm::OnCreate( const adportable::Configuration& config )
{
    config_ = config;
}

void
CentroidForm::OnInitialUpdate()
{
	const adcontrols::CentroidMethod& method = *pMethod_;

    // Scan Type
    ui->doubleSpinBox_peakwidth->setValue( method.rsTofInDa() );
    ui->doubleSpinBox_mz->setValue( method.rsTofAtMz() );
	ui->doubleSpinBox_proportional->setValue( method.rsPropoInPpm() );
	ui->doubleSpinBox_constant->setValue( method.rsConstInDa() );

    ui->doubleSpinBox_peakwidth->setDisabled( true );
    ui->doubleSpinBox_mz->setDisabled( true );
	ui->doubleSpinBox_proportional->setDisabled( true );
    ui->doubleSpinBox_constant->setDisabled( true );

	if ( method.peakWidthMethod() == adcontrols::CentroidMethod::ePeakWidthConstant ) {
		ui->radioButton_constant->setChecked( true );
		ui->doubleSpinBox_constant->setDisabled( false );
	} else if ( method.peakWidthMethod() == adcontrols::CentroidMethod::ePeakWidthProportional ) {
        ui->radioButton_propo->setChecked( true );
        ui->doubleSpinBox_mz->setDisabled( true );
	} else if ( method.peakWidthMethod() == adcontrols::CentroidMethod::ePeakWidthTOF ) {
		ui->radioButton_tof->setChecked( true );
		ui->doubleSpinBox_peakwidth->setDisabled( false );
		ui->doubleSpinBox_mz->setDisabled( false );
	}

	ui->doubleSpinBox_baselinewidth->setValue( method.baselineWidth() );

    if ( method.centroidAreaIntensity() )
		ui->radioButton_area->setChecked( true );
	else
		ui->radioButton_height->setChecked( true );

	ui->doubleSpinBox_centroidfraction->setValue( method.peakCentroidFraction() * 100 );
	ui->doubleSpinBox_baselinewidth->setValue( method.baselineWidth() );
}

void
CentroidForm::OnFinalClose()
{
}

bool
CentroidForm::getContents( boost::any& ) const
{
    return false;
}

bool
CentroidForm::setContents( boost::any& )
{
    return false;
}

void
CentroidForm::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = this;
}

///
void
CentroidForm::update_data()
{
	adcontrols::CentroidMethod& method = *pMethod_;

    // Scan Type
	method.rsTofInDa( ui->doubleSpinBox_peakwidth->value() );
	method.rsTofAtMz( ui->doubleSpinBox_mz->value() );
	method.rsPropoInPpm( ui->doubleSpinBox_proportional->value() );
	method.rsConstInDa( ui->doubleSpinBox_constant->value() ); 

	if ( ui->radioButton_constant->isChecked() )
		method.peakWidthMethod( adcontrols::CentroidMethod::ePeakWidthConstant );
	else if ( ui->radioButton_propo->isChecked() )
	    method.peakWidthMethod( adcontrols::CentroidMethod::ePeakWidthConstant );
	else if ( ui->radioButton_tof->isChecked() )
		method.peakWidthMethod( adcontrols::CentroidMethod::ePeakWidthTOF );

    method.baselineWidth( ui->doubleSpinBox_baselinewidth->value() );
	method.centroidAreaIntensity( ui->radioButton_area->isChecked() );
	method.peakCentroidFraction( ui->doubleSpinBox_centroidfraction->value() / 100.0 );
}

void
CentroidForm::getContents( adcontrols::ProcessMethod& pm )
{
    update_data();
    pm.appendMethod< adcontrols::CentroidMethod >( *pMethod_ );
}

QSize
CentroidForm::sizeHint() const
{
    return QSize( 300, 250 );
}

void
CentroidForm::update()
{
	if ( ui->radioButton_constant->isChecked() ) {
		ui->doubleSpinBox_peakwidth->setDisabled( true );
		ui->doubleSpinBox_mz->setDisabled( true );
		ui->doubleSpinBox_proportional->setDisabled( true );
		ui->doubleSpinBox_constant->setDisabled( false );
	} else if ( ui->radioButton_propo->isChecked() ) {
		ui->doubleSpinBox_peakwidth->setDisabled( true );
		ui->doubleSpinBox_mz->setDisabled( true );
		ui->doubleSpinBox_proportional->setDisabled( false );
		ui->doubleSpinBox_constant->setDisabled( true );
	} else if ( ui->radioButton_tof->isChecked() ) {
		ui->doubleSpinBox_peakwidth->setDisabled( false );
		ui->doubleSpinBox_mz->setDisabled( false );
		ui->doubleSpinBox_proportional->setDisabled( true );
		ui->doubleSpinBox_constant->setDisabled( true );
	}
}
