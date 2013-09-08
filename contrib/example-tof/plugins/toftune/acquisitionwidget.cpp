/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "acquisitionwidget.hpp"
#include <tofinterface/method.hpp>
#include <tofspectrometer/constants.hpp>
#include "ui_acquisitionwidget.h"
#include <adinterface/controlmethodaccessor.hpp>
#include <adportable/serializer.hpp>
#include <iostream>

using namespace toftune;

AcquisitionWidget::AcquisitionWidget(QWidget *parent) : QWidget(parent)
                                                      , ui(new Ui::AcquisitionWidget)
{
    ui->setupUi(this);
    ui->spinBoxSampIntval->setKeyboardTracking( false );
    ui->spinBoxRP->setKeyboardTracking( false );

    connect( ui->spinBoxSampIntval, SIGNAL( valueChanged( int ) ), this, SLOT( onSampIntvalChanged( int ) ) );
    connect( ui->spinBoxRP, SIGNAL( valueChanged( int ) ), this, SLOT( onSampIntvalChanged( int ) ) );
    connect( ui->spinBoxAverage, SIGNAL( valueChanged( int ) ), this, SLOT( onNumAverageChanged( int ) ) );
}

AcquisitionWidget::~AcquisitionWidget()
{
    delete ui;
}

void
AcquisitionWidget::onResolvingPowerChanged( int )
{
    if ( ! isInProgress() )
        emit dataChanged( this );
}

void
AcquisitionWidget::onSampIntvalChanged( int )
{
    if ( ! isInProgress() )
        emit dataChanged( this );
}

void
AcquisitionWidget::onNumAverageChanged( int )
{
    if ( ! isInProgress() )
        emit dataChanged( this );
}

void
AcquisitionWidget::setMethod( const tof::ControlMethod& method )
{
    dataMediator::progress_lock lock( *this );

    ui->spinBoxSampIntval->setValue( method.analyzer.sampling_interval );
    ui->spinBoxRP->setValue( method.analyzer.resolving_power );
    ui->spinBoxAverage->setValue( method.analyzer.number_of_average );
}

void
AcquisitionWidget::getMethod( tof::ControlMethod& method ) const
{
    method.analyzer.resolving_power = ui->spinBoxRP->value();
    method.analyzer.sampling_interval = ui->spinBoxSampIntval->value();
    method.analyzer.number_of_average = ui->spinBoxAverage->value();
}


void
AcquisitionWidget::OnCreate( const adportable::Configuration& )
{
}

void
AcquisitionWidget::OnInitialUpdate()
{
}

void
AcquisitionWidget::OnFinalClose()
{
}

void
AcquisitionWidget::onUpdate( boost::any& )
{
}

bool
AcquisitionWidget::getContents( boost::any& a ) const
{
	using namespace adportable;
    using adinterface::ControlMethodAccessorT;
    using tofspectrometer::constants::C_INSTRUMENT_NAME;

	ControlMethodAccessorT< tof::ControlMethod
                            , serializer< tof::ControlMethod > > accessor( C_INSTRUMENT_NAME );
	tof::ControlMethod im;
	if ( accessor.getMethod( im, a ) ) {
		getMethod( im );
		accessor.setMethod( a, im );
		return true;
	}
	return false;
}

bool
AcquisitionWidget::setContents( boost::any& a )
{
	using namespace adportable;
	using adinterface::ControlMethodAccessor;
    using adinterface::ControlMethodAccessorT;
    using tofspectrometer::constants::C_INSTRUMENT_NAME;

	if ( ControlMethodAccessor::isReference( a ) ) {
		ControlMethodAccessorT< tof::ControlMethod
                                , adportable::serializer< tof::ControlMethod > > accessor( C_INSTRUMENT_NAME );
		tof::ControlMethod im;
		accessor.getMethod( im, a );
		setMethod( im );
		return true;
	}
	return false;

}
