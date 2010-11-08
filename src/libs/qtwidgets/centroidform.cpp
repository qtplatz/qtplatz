//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "centroidform.h"
#include "ui_centroidform.h"
#include <adcontrols/centroidmethod.h>
#include <QStandardItemModel>

using namespace qtwidgets;

CentroidForm::CentroidForm(QWidget *parent) : QWidget(parent)
                                            , ui(new Ui::CentroidForm)
                                            , model_( new QStandardItemModel )
                                            , method_( new adcontrols::CentroidMethod ) 
{
    ui->setupUi(this);
    ui->treeView->setModel( model_.get() );
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
    update_model();
}

void
CentroidForm::OnFinalClose()
{
}

///
void
CentroidForm::update_model()
{
    QStandardItemModel& model = *model_;
    adcontrols::CentroidMethod& method = *method_;
    // require 4 rows
    QStandardItem * rootNode = model.invisibleRootItem();

    if ( model.rowCount() == 0 ) {

        rootNode->setColumnCount(2);

        QStandardItem * scanType = new QStandardItem( "ScanType" );
        rootNode->appendRow( scanType );
        do {
            scanType->appendRow( new QStandardItem( "Peak Width [Da]" ) );
            scanType->appendRow( new QStandardItem( "Proportional [ppm]" ) );
            scanType->appendRow( new QStandardItem( "Constant [Da]" ) );
        } while(0);

        model.insertColumn( 1, scanType->index() );

        ui->treeView->expand( scanType->index() );
        ui->treeView->setColumnWidth( 0, 200 );

        rootNode->appendRow( new QStandardItem( "Area/Height" ) );
        rootNode->appendRow( new QStandardItem( "Baseline Width [Da]" ) );
        rootNode->appendRow( new QStandardItem( "Peak Centroid Fraction [%]" ) );
    }

    model.setData( model.index( 0, 1, model.item( 0, 0 )->index() ), method.rsTofInDa() );
    model.setData( model.index( 1, 1, model.item( 0, 0 )->index() ), method.rsPropoInPpm() );
    model.setData( model.index( 2, 1, model.item( 0, 0 )->index() ), method.rsConstInDa() );
    
    model.setItem( 0, 1, new QStandardItem( "TOF" ) );
    model.setItem( 1, 1, new QStandardItem( method.centroidAreaIntensity() ? "Area" : "Height" ) );

    model.setData( model.index( 2, 1 ), method.baselineWidth() );
    model.setData( model.index( 3, 1 ), method.peakCentroidFraction() * 100 );
}