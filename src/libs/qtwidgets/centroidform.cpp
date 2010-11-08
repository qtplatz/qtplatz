//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "centroidform.h"
#include "ui_centroidform.h"
//#include "standardmodel.h"
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
    // require 4 rows
    QStandardItem * rootNode = model.invisibleRootItem();

    if ( model.rowCount() == 0 ) {
        //QList< QStandardITem * > rowItems;
        //rowItems << new QStandardItem( "Centroid" );
        //rowItems << new QStandardItem( "value" );
        rootNode->setColumnCount(2);

        QStandardItem * scanType = new QStandardItem( "ScanType" );
        rootNode->appendRow( scanType );
        ui->treeView->expand( scanType->index() );
        
        do {
            scanType->appendRow( new QStandardItem( "Peak Width [Da]" ) );
            scanType->appendRow( new QStandardItem( "Proportional [ppm]" ) );
            scanType->appendRow( new QStandardItem( "Constant [Da]" ) );
        } while(0);

        rootNode->appendRow( new QStandardItem( "Area/Height" ) );
        rootNode->appendRow( new QStandardItem( "Baseline Width [Da]" ) );
        rootNode->appendRow( new QStandardItem( "Peak Centroid Fraction [%]" ) );
    }
    model.setItem( 0, 1, new QStandardItem( "xxx" ) );
    model.setItem( 1, 1, new QStandardItem( "yyyy" ) );
/*
    int row = 0;
    QModelIndex parent = model.index( row, 0 ); // parent
    model.setData( parent, "ScanType" );
    // choice of "TOF | Proportional | Constant"
    model.setData( model.index( row, 1 ), "TOF" );

    do {
        int childRow = model.rowCount( parent );
        if ( childRow < 3 )
            model.insertRows( 0, 3 - childRow, parent );
        //
        model.setData( model.index( 0, 0, parent ), "Peak Width [Da]" );
        model.setData( model.index( 0, 1, parent ), "0.10000" );      
        //
        model.setData( model.index( 1, 0, parent ), "Proportional[ppm]" );
        model.setData( model.index( 1, 1, parent ), "200.0000" );
        //
        model.setData( model.index( 2, 0, parent ), "Constant[Da]" );
        model.setData( model.index( 2, 1, parent ), "1.0000" );

        ui->treeView->expand( parent );
    } while(0);
    ui->treeView->setColumnWidth( 0, 200 );

    //
    model.setData( model.index( ++row, 0 ), "Area/Height" );
    model.setData( model.index( row, 1 ), "Area" );
    // 
    model.setData( model.index( ++row, 0 ), "Baseline Width [Da]" );
    model.setData( model.index( row, 1 ), "500.0000" );
    // 
    model.setData( model.index( ++row, 0 ), "Peak Centroid Fraction [%]" );
    model.setData( model.index( row, 1 ), "50.0000" );
*/
}