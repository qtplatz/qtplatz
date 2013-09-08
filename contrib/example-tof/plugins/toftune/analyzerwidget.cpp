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

#include "analyzerwidget.hpp"
#include "ui_analyzerwidget.h"
#include "doublespinslider.hpp"
#include <tofspectrometer/constants.hpp>
#include <tofinterface/method.hpp>
#include <adinterface/controlmethodaccessor.hpp>
#include <adportable/serializer.hpp>

using namespace toftune;

AnalyzerWidget::AnalyzerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnalyzerWidget)
{
    ui->setupUi(this);

    typedef Interactor< DoubleSpinSlider > X;

    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox,  ui->horizontalSlider, 0.0, 2500, ui->label ) ) ); // RF(V)
    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox1, ui->horizontalSlider1, 0.0, 2500, ui->label1 ) ) ); // RF(MHz)
    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox2, ui->horizontalSlider2, 0.0, 2500, ui->label2 ) ) ); // RF(A)
    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox3, ui->horizontalSlider3, 0.0, 2500, ui->label3 ) ) ); // RF Amp Temp(C)
    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox4, ui->horizontalSlider4, 0.0, 2500, ui->label4 ) ) ); // DC+(V)
    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox5, ui->horizontalSlider5, 0.0, 2500, ui->label5 ) ) ); // DC-(V)

    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox10,  ui->horizontalSlider10, 0.0, 2500, ui->label10 ) ) ); // Post filter DC(V)
    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox11,  ui->horizontalSlider11, 0.0, 2500, ui->label11 ) ) ); // Exit electrode (V)

    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox20, ui->horizontalSlider20, 0.0, 2500, ui->label2 ) ) ); // RF(A)
    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox21, ui->horizontalSlider21, 0.0, 2500, ui->label3 ) ) ); // RF Amp Temp(C)
    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox22, ui->horizontalSlider22, 0.0, 2500, ui->label4 ) ) ); // DC+(V)
}

AnalyzerWidget::~AnalyzerWidget()
{
    delete ui;
}

void
AnalyzerWidget::setMethod( const tof::ControlMethod& )
{
}

void
AnalyzerWidget::getMethod( tof::ControlMethod& ) const
{
}


void
AnalyzerWidget::OnCreate( const adportable::Configuration& )
{
}

void
AnalyzerWidget::OnInitialUpdate()
{
}

void
AnalyzerWidget::OnFinalClose()
{
}

void
AnalyzerWidget::onUpdate( boost::any& )
{
}

bool
AnalyzerWidget::getContents( boost::any& a ) const
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
AnalyzerWidget::setContents( boost::any& a )
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
