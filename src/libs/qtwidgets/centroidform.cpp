//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "centroidform.h"
#include "ui_centroidform.h"
#include <adcontrols/centroidmethod.h>
#include <QStandardItemModel>
#include "centroiddelegate.h"
#include "standarditemhelper.h"
using namespace qtwidgets;

/////////////////////

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
    QStandardItemModel& model = *model_;
    //adcontrols::CentroidMethod& method = *method_;
    // require 4 rows
    QStandardItem * rootNode = model.invisibleRootItem();

    delegate_.reset( new CentroidDelegate(this) );

    ui->treeView->setItemDelegate( delegate_.get() );

    rootNode->setColumnCount(2);
    model.setHeaderData( 0, Qt::Horizontal, "Centroid" );

    QStandardItem * scanType =
        StandardItemHelper::appendRow( rootNode, "ScanType", qVariantFromValue( CentroidDelegate::PeakWidthMethod( method_->peakWidthMethod() ) ) );
    //QStandardItem * scanType = new QStandardItem( "ScanType" );
    //scanType->setEditable( false );
    //rootNode->appendRow( scanType );
    do {
        StandardItemHelper::appendRow( scanType, "Peak Width [Da]" );
        StandardItemHelper::appendRow( scanType, "Proportional [ppm]" );
        StandardItemHelper::appendRow( scanType, "Constant [Da]" );
        //QStandardItem * item = new QStandardItem( "Peak Width [Da]" );
        //item->setEditable( false );
        // scanType->appendRow( item );
        //item = new QStandardItem( "Proportional [ppm]" );
        //item->setEditable( false );
        //scanType->appendRow( item );
        //item = new QStandardItem( "Constant [Da]" );
        //item->setEditable( false );        
        //scanType->appendRow( item );
    } while(0);

    model.insertColumn( 1, scanType->index() );

    ui->treeView->expand( scanType->index() );
    ui->treeView->setColumnWidth( 0, 200 );

    rootNode->appendRow( new QStandardItem( "Area/Height" ) );
    rootNode->appendRow( new QStandardItem( "Baseline Width [Da]" ) );
    rootNode->appendRow( new QStandardItem( "Peak Centroid Fraction [%]" ) );

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

    model.setData( model.index( 0, 1), qVariantFromValue( CentroidDelegate::PeakWidthMethod( method.peakWidthMethod() ) ) );

    model.setData( model.index( 0, 1, model.item( 0, 0 )->index() ), method.rsTofInDa() );
    model.setData( model.index( 1, 1, model.item( 0, 0 )->index() ), method.rsPropoInPpm() );
    model.setData( model.index( 2, 1, model.item( 0, 0 )->index() ), method.rsConstInDa() );
    
    do {
        QStandardItem * item = model.itemFromIndex( model.index( 0, 0, model.item( 0, 0 )->index() ) );
        item->setEnabled( false );
        item = model.itemFromIndex( model.index( 0, 1, model.item( 0, 0 )->index() ) );
        item->setEnabled( false );
    } while(0);

    model.setData( model.index( 1, 1 ), qVariantFromValue( CentroidDelegate::AreaHeight( method.centroidAreaIntensity() ) ) );

    model.setData( model.index( 2, 1 ), method.baselineWidth() );
    model.setData( model.index( 3, 1 ), method.peakCentroidFraction() * 100 );
}
