/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adportable/debug.hpp>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace adwidgets;

MSToleranceForm::MSToleranceForm(QWidget *parent) : QWidget(parent), ui( new Ui::MSToleranceForm )
{
    ui->setupUi(this);

    ui->radioButton_area->setChecked( true );
    ui->radioButton_intens->setChecked( true );
    ui->radioButton_2->setChecked( true );

    ui->radioButton_intens->setEnabled( false );
    ui->radioButton_closest->setEnabled( false );
    ui->label->setText( "Width(mDa)" );

    connect( ui->radioButton_centroid, &QRadioButton::toggled, this, [&](bool checked){
            ui->radioButton_intens->setEnabled( checked );
            ui->radioButton_closest->setEnabled( checked );
            ui->label->setText( checked ? "Tolerance(mDa)" : "Width(mDa)" );
        });

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

bool
MSToleranceForm::setContents( const adcontrols::TargetingMethod& tm )
{
    adcontrols::QuanResponseMethod m;
    m.setWidth( tm.tolerance( adcontrols::idToleranceDaltons ) );
    m.setFindAlgorithm( tm.findAlgorithm() );
    return setContents( m );
}

bool
MSToleranceForm::setContents( const adcontrols::QuanResponseMethod& m )
{
    using namespace adcontrols;

    ui->doubleSpinBoxWidth->setValue( m.width() * 1000 );

    if ( m.intensityMethod() == QuanResponseMethod::idArea )
        ui->radioButton_area->setChecked( true );
    else
        ui->radioButton_centroid->setChecked( true );

    if ( m.findAlgorithm() == idFindLargest )
        ui->radioButton_intens->setChecked( true );
    else
        ui->radioButton_closest->setChecked( true );

    if ( m.dataSelectionMethod() == QuanResponseMethod::idAverage )
        ui->radioButton_2->setChecked( true );
    else
        ui->radioButton_3->setChecked( true );

    return true;
}

bool
MSToleranceForm::getContents( adcontrols::QuanResponseMethod& m ) const
{
    using namespace adcontrols;

    m.setWidthMethod( QuanResponseMethod::idWidthDaltons );
    m.setWidth( ui->doubleSpinBoxWidth->value() / 1000.0 );
    m.setIntensityMethod( ui->radioButton_centroid->isChecked() ? QuanResponseMethod::idCentroid : QuanResponseMethod::idArea );
    m.setFindAlgorithm( ui->radioButton_closest->isChecked() ? idFindClosest : idFindLargest );
    m.setDataSelectionMethod( ui->radioButton_2->isChecked() ? QuanResponseMethod::idAverage : QuanResponseMethod::idLargest );

    return true;
}

std::string
MSToleranceForm::toJson( bool pritty ) const
{
    adcontrols::QuanResponseMethod m;
    getContents( m );
    return m.toJson();
}

void
MSToleranceForm::fromJson( const std::string& json )
{
    adcontrols::QuanResponseMethod m;
    m.fromJson( json );
    setContents( m );
}
