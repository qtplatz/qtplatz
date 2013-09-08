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

#include "ionsourcewidget.hpp"
#include "ui_ionsourcewidget.h"
#include "doublespinslider.hpp"
#include "interactor.hpp"
#include <tofspectrometer/constants.hpp>
#include <tofinterface/method.hpp>
#include <adinterface/controlmethodaccessor.hpp>
#include <adportable/serializer.hpp>
#include <boost/any.hpp>

using namespace toftune;

IonSourceWidget::IonSourceWidget(QWidget *parent) : QWidget(parent)
                                                  , ui(new Ui::IonSourceWidget)
{
    ui->setupUi(this);

    typedef Interactor< DoubleSpinSlider > X;

    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox_01,  ui->horizontalSlider_01, 0.0, 2500, ui->label01 ) ) ); // Housing temp(C)
    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox_02,  ui->horizontalSlider_02, 0.0, 2500, ui->label02 ) ) ); // GC I/F (C)

    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox11,  ui->horizontalSlider11, 0.0, 2500, ui->label11 ) ) ); // Ionization (V)
    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox12,  ui->horizontalSlider12, 0.0, 2500, ui->label12 ) ) ); // Filament (mA)
    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox13,  ui->horizontalSlider13, 0.0, 2500, ui->label13 ) ) ); // Filament (V)
    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox14,  ui->horizontalSlider14, 0.0, 2500, ui->label14 ) ) ); // Trap(V)
    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox15,  ui->horizontalSlider15, 0.0, 2500, ui->label15 ) ) ); // Trap(mA)
    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox16,  ui->horizontalSlider16, 0.0, 2500, ui->label16 ) ) ); // Chamber (uA)

    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox21,  ui->horizontalSlider21, 0.0, 2500, ui->label21 ) ) ); // Pullout(V)
    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox22,  ui->horizontalSlider22, 0.0, 2500, ui->label22 ) ) ); // Lens 1 (V)
    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox23,  ui->horizontalSlider23, 0.0, 2500, ui->label23 ) ) ); // Lens 2 (V)
    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox24,  ui->horizontalSlider24, 0.0, 2500, ui->label24 ) ) ); // Lens 3 (V)
    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox25,  ui->horizontalSlider25, 0.0, 2500, ui->label25 ) ) ); // Lens 4 (V)
    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox26,  ui->horizontalSlider26, 0.0, 2500, ui->label26 ) ) ); // Lens 5 (V)
    interactor_vec.push_back( X(new DoubleSpinSlider( ui->doubleSpinBox27,  ui->horizontalSlider27, 0.0, 2500, ui->label27 ) ) ); // Pre-filter DC (V)
}

IonSourceWidget::~IonSourceWidget()
{
    delete ui;
}

void
IonSourceWidget::setMethod( const tof::ControlMethod& m )
{
	dataMediator::progress_lock lock( *const_cast<IonSourceWidget *>(this) );

    if ( m.ionSource.type() == typeid( tof::IonSource_EI_Method ) ) {
		const tof::IonSource_EI_Method& ei = boost::get< tof::IonSource_EI_Method >( m.ionSource );

		typedef Interactor< DoubleSpinSlider > X;

		boost::get< X >( interactor_vec[0] ).ptr_.get()->value( ei.source_temp );
		boost::get< X >( interactor_vec[1] ).ptr_.get()->value( ei.interface_temp );
	}
}

void
IonSourceWidget::getMethod( tof::ControlMethod& m ) const
{
    dataMediator::progress_lock lock( *const_cast<IonSourceWidget *>(this) );

    tof::IonSource_EI_Method ei;

    typedef Interactor<DoubleSpinSlider> X;

	ei.source_temp = boost::get< X >( interactor_vec[ 0 ] ).ptr_.get()->value();
	ei.interface_temp = boost::get< X >( interactor_vec[ 1 ] ).ptr_.get()->value();

	m.ionSource = ei;
}

// LifeCycle
void
IonSourceWidget::OnCreate( const adportable::Configuration& )
{
}

void
IonSourceWidget::OnInitialUpdate()
{
}

void
IonSourceWidget::OnFinalClose()
{
}

void
IonSourceWidget::onUpdate( boost::any& )
{
}

bool
IonSourceWidget::getContents( boost::any& a ) const
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
IonSourceWidget::setContents( boost::any& a )
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
