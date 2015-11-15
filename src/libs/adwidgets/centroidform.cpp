/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include <adportable/is_type.hpp>
#include <adlog/logger.hpp>
#include <QStandardItemModel>
#include <boost/any.hpp>

using namespace adwidgets;

/////////////////////

CentroidForm::CentroidForm(QWidget *parent) : QWidget(parent)
                                            , ui(new Ui::CentroidForm)
                                            , pMethod_( new adcontrols::CentroidMethod ) 
{
	ui->setupUi(this);
    QStringList areaMethods;
    areaMethods << "Intens. x mDa" << "Intens. x Time(ns)" << "Width Norm." << "Samp. Interval";
    ui->comboBox->addItems( areaMethods );
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
    update_data( *pMethod_ );
}

void
CentroidForm::OnFinalClose()
{
}

bool
CentroidForm::getContents( boost::any& any ) const
{
    if ( ! adportable::a_type< adcontrols::ProcessMethod >::is_pointer( any ) )
        return false;

    adcontrols::ProcessMethod* pm = boost::any_cast< adcontrols::ProcessMethod* >( any );
    const_cast< CentroidForm *>(this)->update_data();
    pm->appendMethod< adcontrols::CentroidMethod >( *pMethod_ );
    
    return true;
}

bool
CentroidForm::setContents( boost::any& any )
{
    if ( ! adportable::a_type< adcontrols::ProcessMethod >::is_a( any ) )
        return false;

    const adcontrols::ProcessMethod& pm = boost::any_cast< adcontrols::ProcessMethod& >( any );
    const adcontrols::CentroidMethod * t = pm.find< adcontrols::CentroidMethod >();
    if ( ! t )
        return false;
    *pMethod_ = *t;
    update_data( *pMethod_ );
    return true;
}

///
void
CentroidForm::update_data( const adcontrols::CentroidMethod& method )
{
    scoped_lock lock( *this );

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

	//ui->doubleSpinBox_baselinewidth->setValue( method.baselineWidth() );

    if ( method.centroidAreaIntensity() )
		ui->radioButton_area->setChecked( true );
	else
		ui->radioButton_height->setChecked( true );

	ui->doubleSpinBox_centroidfraction->setValue( method.peakCentroidFraction() * 100 );
	//ui->doubleSpinBox_baselinewidth->setValue( method.baselineWidth() );
	using adcontrols::CentroidMethod;

	ui->noiseFilterMethod->setCheckState( method.noiseFilterMethod() == CentroidMethod::eNoFilter ? Qt::Unchecked : Qt::Checked );
	ui->cutoffMHz->setValue( method.cutoffFreqHz() / 1.0e6 + 0.5 );

    ui->comboBox->setCurrentIndex( method.areaMethod() );
}

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

    //method.baselineWidth( ui->doubleSpinBox_baselinewidth->value() );
	method.centroidAreaIntensity( ui->radioButton_area->isChecked() );
	method.peakCentroidFraction( ui->doubleSpinBox_centroidfraction->value() / 100.0 );
	
	method.noiseFilterMethod( ui->noiseFilterMethod->isChecked() ?
		adcontrols::CentroidMethod::eDFTLowPassFilter : adcontrols::CentroidMethod::eNoFilter );

	method.cutoffFreqHz( ui->cutoffMHz->value() * 1.0e6 );

    method.areaMethod( adcontrols::CentroidMethod::eAreaMethod( ui->comboBox->currentIndex() ) );
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
    return QSize( 274, 196 );
}

void
CentroidForm::update()
{
    scoped_lock lock( *this );

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

void 
CentroidForm::on_doubleSpinBox_peakwidth_valueChanged(double arg1)
{
    (void)arg1;
    if ( ! isScoped() )
        emit valueChanged();
}

void 
CentroidForm::on_doubleSpinBox_centroidfraction_valueChanged(double arg1)
{
	(void)arg1;
    if ( ! isScoped() )
        emit valueChanged();
}

void 
CentroidForm::on_noiseFilterMethod_stateChanged(int arg1)
{
	(void)arg1;
    if ( ! isScoped() )
        emit valueChanged();
}

void 
CentroidForm::on_cutoffMHz_valueChanged(int arg1)
{
	(void)arg1;
    if ( ! isScoped() )
        emit valueChanged();
}
