/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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

#include "mscalibrationform.h"
#include "ui_mscalibrationform.h"
#include "mscalibratedelegate.h"

#include <adcontrols/mscalibratemethod.h>
#include <adcontrols/msreferencedefns.h>
#include <adcontrols/processmethod.h>
#include <adportable/configuration.h>
#include <QStandardItemModel>
#include "standarditemhelper.h"

using namespace qtwidgets;

MSCalibrationForm::MSCalibrationForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MSCalibrationForm)
    , pModel_( new QStandardItemModel )
    , pConfig_( new adportable::Configuration )
    , pMethod_( new adcontrols::MSCalibrateMethod ) 
    , pDelegate_( new MSCalibrateDelegate )
{
    ui->setupUi(this);
    ui->treeView->setModel( pModel_.get() );
    ui->treeView->setItemDelegate( pDelegate_.get() );
}

MSCalibrationForm::~MSCalibrationForm()
{
    delete ui;
}

void
MSCalibrationForm::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;
}

void
MSCalibrationForm::OnInitialUpdate()
{
    QStandardItemModel& model = *pModel_;
    adcontrols::MSCalibrateMethod& method = *pMethod_;

    QStandardItem * rootNode = model.invisibleRootItem();
    ui->treeView->setItemDelegate( pDelegate_.get() );

    rootNode->setColumnCount(2);
    model.setHeaderData( 0, Qt::Horizontal, "MSCaribrate" );
    model.setHeaderData( 1, Qt::Horizontal, "" );

//----
    StandardItemHelper::appendRow( rootNode, "Polynomial[degree]", method.polynomialDegree() );
    StandardItemHelper::appendRow( rootNode, "Mass Tolerance[Da]", method.massToleranceDa() );
    StandardItemHelper::appendRow( rootNode, "Minimum RA[%]",      method.minimumRAPercent() );
    StandardItemHelper::appendRow( rootNode, "Low Mass[Da]",       method.lowMass() );
    StandardItemHelper::appendRow( rootNode, "High Mass[Da]",      method.highMass() );

    //------ create Xe reference -------
    adcontrols::MSReferenceDefns Xe;
    Xe.addFormula( adcontrols::MSRefFormula( L"126Xe", true ) );
    Xe.addFormula( adcontrols::MSRefFormula( L"128Xe", true ) );
    Xe.addFormula( adcontrols::MSRefFormula( L"129Xe", false ) );
    Xe.addFormula( adcontrols::MSRefFormula( L"130Xe", false ) );
    Xe.addFormula( adcontrols::MSRefFormula( L"131Xe", false ) );
    Xe.addFormula( adcontrols::MSRefFormula( L"132Xe", false ) );
    Xe.addFormula( adcontrols::MSRefFormula( L"134Xe", false ) );
    Xe.addFormula( adcontrols::MSRefFormula( L"136Xe", true ) );
    // ---------------------------------

    QStandardItem * refItem = StandardItemHelper::appendRow( rootNode, "Mass References", qVariantFromValue( MSCalibrateDelegate::MSReferences() ) );

    ui->treeView->setColumnWidth( 0, 200 );
}

void
MSCalibrationForm::OnFinalClose()
{
}

void
MSCalibrationForm::getContents( adcontrols::ProcessMethod& pm )
{
    pm.appendMethod< adcontrols::MSCalibrateMethod >( *pMethod_ );
}