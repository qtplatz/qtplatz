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

#include "mstoleranceform.hpp"
#include "ui_mstoleranceform.h"
#include <adcontrols/targetingmethod.hpp>

using namespace adwidgets;

MSToleranceForm::MSToleranceForm(QWidget *parent) : QWidget(parent), ui( new Ui::MSToleranceForm )
{
    ui->setupUi(this);
}

MSToleranceForm::~MSToleranceForm()
{
    delete ui;
}

void
MSToleranceForm::setTitle( const QString& title )
{
    ui->groupBox->setTitle( title );
}

bool
MSToleranceForm::isChecked() const
{
    return ui->groupBox->isChecked();
}

void
MSToleranceForm::setChecked( bool checked )
{
    ui->groupBox->setChecked( checked );
}


MSToleranceForm::idWidthMethod
MSToleranceForm::widthMethod()
{
    if ( ui->radioButtonRP->isChecked() )
        return idWidthRP;
    return idWidthDaltons;
}

void
MSToleranceForm::setWidthMethod( idWidthMethod which )
{
    if ( which == idWidthRP )
        ui->radioButtonRP->setChecked( true );
    else
        ui->radioButtonWidth->setChecked( true );
}


double
MSToleranceForm::value( idWidthMethod which ) const
{
    if ( which == idWidthRP )
        return ui->doubleSpinBoxRP->value();
    else
        return ui->doubleSpinBoxWidth->value();
}

void
MSToleranceForm::setValue( idWidthMethod which , double value )
{
    if ( which == idWidthRP )
        ui->doubleSpinBoxRP->setValue( value );
    else
        ui->doubleSpinBoxWidth->setValue( value );
}

bool
MSToleranceForm::setContents( const adcontrols::TargetingMethod& tm )
{
    ui->groupBox->setCheckable( false );
    ui->groupBox->setChecked( true );
    setWidthMethod( tm.toleranceMethod() == adcontrols::idTolerancePpm ? idWidthRP : idWidthDaltons );
    setValue( idWidthRP, tm.tolerance( adcontrols::idTolerancePpm ) );
    setValue( idWidthDaltons, tm.tolerance( adcontrols::idToleranceDaltons ) * 1000 ); // Da -> mDa
    return true;
}

bool
MSToleranceForm::getContents( adcontrols::TargetingMethod& tm )
{
    if ( widthMethod() == idWidthRP )
        tm.setToleranceMethod( adcontrols::idTolerancePpm );
    else
        tm.setToleranceMethod( adcontrols::idToleranceDaltons );

    tm.setTolerance( adcontrols::idTolerancePpm, value( idWidthRP ) );
    tm.setTolerance( adcontrols::idToleranceDaltons, value( idWidthDaltons ) / 1000.0 );

    return true;
}
