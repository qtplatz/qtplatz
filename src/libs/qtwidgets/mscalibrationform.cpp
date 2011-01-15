//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "mscalibrationform.h"
#include "ui_mscalibrationform.h"
#include "mscalibratedelegate.h"
#include <adcontrols/mscalibratemethod.h>
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
    StandardItemHelper::appendRow( rootNode, "Minimum RA[%]", method.minimumRAPercent() );
    StandardItemHelper::appendRow( rootNode, "Low Mass[Da]", method.lowMass() );
    StandardItemHelper::appendRow( rootNode, "High Mass[Da]", method.highMass() );

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