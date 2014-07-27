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

#include "quanqueryform.hpp"
#include "ui_quanqueryform.h"
#include <QStringList>

using namespace quan;

QuanQueryForm::QuanQueryForm(QWidget *parent) :  QWidget(parent)
                                              , ui(new Ui::QuanQueryForm)
{
    ui->setupUi(this);
    ui->comboBox->clear();
    ui->comboBox->addItems( QStringList() << "View all (simple)" << "View all" << "Viwe full" << "Standard/Calibration" << "Unknown" );
}

QuanQueryForm::~QuanQueryForm()
{
    delete ui;
}

void
QuanQueryForm::setSQL( const QString& t )
{
    ui->plainTextEdit->clear();
    ui->plainTextEdit->insertPlainText( t );
}

QString
QuanQueryForm::sql() const
{
    return ui->plainTextEdit->toPlainText();
}

void 
QuanQueryForm::on_plainTextEdit_textChanged()
{
}

void 
QuanQueryForm::on_pushButton_pressed()
{
    emit triggerQuery( ui->plainTextEdit->toPlainText() );
}

void 
QuanQueryForm::on_comboBox_currentIndexChanged(int index)
{
    int idx = 0;
    if ( index == idx++ ) { // view simple
        setSQL("\
SELECT dataSource, row, level, formula, mass, intensity, sampletype FROM QuanSample,QuanResponse \
WHERE QuanSample.id = sampleId AND formula like '%' ORDER BY formula" );

    } else if ( index == idx++ ) { // view all

        setSQL("\
SELECT dataSource, row, QuanSample.level, QuanResponse.formula, QuanResponse.mass, QuanResponse.intensity, QuanAmount.amount,sampletype \n\
FROM QuanSample,QuanResponse,QuanAmount\n\
WHERE QuanSample.id = sampleId\n\
AND QuanAmount.CompoundId = (SELECT id from QuanCompound WHERE uniqId = QuanResponse.compoundId)\n\
AND QuanAmount.level = QuanSample.level\n\
AND formula like '%' ORDER BY formula" );

    } else if ( index == idx++ ) { // view full

        setSQL("\
SELECT dataSource, row, QuanSample.level, QuanResponse.formula, QuanCompound.mass AS 'exact mass', QuanResponse.mass, QuanResponse.intensity, QuanAmount.amount,sampletype,QuanCompound.description \n\
FROM QuanSample,QuanResponse,QuanAmount,QuanCompound \n\
WHERE QuanSample.id = sampleId AND QuanCompound.uniqId = QuanResponse.compoundId\n\
AND QuanAmount.CompoundId = (SELECT id from QuanCompound WHERE uniqId = QuanResponse.compoundId)\n\
AND QuanAmount.level = QuanSample.level\n\
AND QuanResponse.formula like '%' ORDER BY Quanresponse.formula");

    } else if ( index == idx++ ) { // Standard/Calibration

        setSQL("\
SELECT QuanResponse.formula, QuanResponse.intensity, QuanAmount.amount,sampletype \n\
FROM QuanSample,QuanResponse,QuanAmount \n\
WHERE QuanSample.id = sampleId AND sampleType = 1\n\
AND QuanAmount.CompoundId = (SELECT id from QuanCompound WHERE uniqId = QuanResponse.compoundId)\n\
AND QuanAmount.level = QuanSample.level\n\
AND QuanResponse.formula like '%' ORDER BY Quanresponse.formula");


    } else if ( index == idx++ ) { // Unknown

        setSQL("\
SELECT QuanResponse.formula, QuanResponse.intensity, sampletype \n\
FROM QuanSample,QuanResponse \n\
WHERE QuanSample.id = sampleId AND sampleType = 0\n\
AND QuanResponse.formula like '%' ORDER BY QuanResponse.formula");

    }


}
