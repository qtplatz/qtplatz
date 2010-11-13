//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "targetingform.h"
#include "ui_targetingform.h"
#include "targetingdelegate.h"
#include <adcontrols/targetingmethod.h>
#include <adportable/configuration.h>
#include <QStandardItemModel>

using namespace qtwidgets;

TargetingForm::TargetingForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TargetingForm)
    , pModel_( new QStandardItemModel )
    , pConfig_( new adportable::Configuration )
    , pMethod_( new adcontrols::TargetingMethod )
    , pDelegate_( new TargetingDelegate ) 
{
    ui->setupUi(this);
    ui->treeView->setModel( pModel_.get() );
    ui->treeView->setItemDelegate( pDelegate_.get() );
}

TargetingForm::~TargetingForm()
{
    delete ui;
}

void
TargetingForm::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;
}

void
TargetingForm::OnInitialUpdate()
{
    QStandardItemModel& model = *pModel_;
    adcontrols::TargetingMethod& method = *pMethod_;

    QStandardItem * rootNode = model.invisibleRootItem();

    ui->treeView->setItemDelegate( pDelegate_.get() );

    rootNode->setColumnCount(2);
    model.setHeaderData( 0, Qt::Horizontal, "Targeting" );
}

void
TargetingForm::OnFinalClose()
{
}
