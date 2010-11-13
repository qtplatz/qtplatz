//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "mslockform.h"
#include "ui_mslockform.h"
#include "mslockdelegate.h"
#include "standarditemhelper.h"
#include <adportable/configuration.h>
#include <QStandardItemModel>

using namespace qtwidgets;

MSLockForm::MSLockForm(QWidget *parent) : QWidget(parent)
                                        , ui(new Ui::MSLockForm)
                                        , pConfig_( new adportable::Configuration )
                                        , pModel_( new QStandardItemModel )
                                        , pDelegate_( new MSLockDelegate )
{
    ui->setupUi(this);
    ui->treeView->setModel( pModel_.get() );
}

MSLockForm::~MSLockForm()
{
    delete ui;
}

void
MSLockForm::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;
}

void
MSLockForm::OnInitialUpdate()
{
    QStandardItemModel& model = *pModel_;
    // adcontrols::CentroidMethod& method = *pMethod_;

    QStandardItem * rootNode = model.invisibleRootItem();

    ui->treeView->setItemDelegate( pDelegate_.get() );

    rootNode->setColumnCount(2);
    model.setHeaderData( 0, Qt::Horizontal, "Lock mass" );
/*
    QStandardItem * scanType =
        StandardItemHelper::appendRow( rootNode, "Algorithm", qVariantFromValue( CentroidDelegate::PeakWidthMethod( method_->peakWidthMethod() ) ) );

    do {
        StandardItemHelper::appendRow( scanType, "Peak Width [Da]" );
        model.insertColumn( 1, scanType->index() );
        model.setData( model.index( 0, 1, model.item( 0, 0 )->index() ), method.rsTofInDa() );

        StandardItemHelper::appendRow( scanType, "Proportional [ppm]", method.rsPropoInPpm() );
        StandardItemHelper::appendRow( scanType, "Constant [Da]", method.rsConstInDa() );
    } while(0);

    StandardItemHelper::appendRow( rootNode, "Area/Height", qVariantFromValue( CentroidDelegate::AreaHeight( method.centroidAreaIntensity() ) ) );
    StandardItemHelper::appendRow( rootNode, "Baseline Width [Da]", method.baselineWidth() );
    StandardItemHelper::appendRow( rootNode, "Peak Centroid Fraction [%]", method.peakCentroidFraction() * 100 );

    // update_model();
    
    //--------------
    ui->treeView->expand( scanType->index() );
    ui->treeView->setColumnWidth( 0, 200 );
*/
}

void
MSLockForm::OnFinalClose()
{
}
