//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "isotopeform.h"
#include "ui_isotopeform.h"
#include "isotopedelegate.h"
#include "standarditemhelper.h"
#include <adcontrols/isotopemethod.h>
#include <adcontrols/processmethod.h>
#include <adportable/configuration.h>
#include <QStandardItemModel>

using namespace qtwidgets;

IsotopeForm::IsotopeForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IsotopeForm)
    , pModel_( new QStandardItemModel )
    , pConfig_( new adportable::Configuration )
    , pMethod_( new adcontrols::IsotopeMethod ) 
{
    ui->setupUi(this);
    ui->treeView->setModel( pModel_.get() );
}

IsotopeForm::~IsotopeForm()
{
    delete ui;
}

void
IsotopeForm::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;
}

void
IsotopeForm::OnInitialUpdate()
{
    QStandardItemModel& model = *pModel_;
    adcontrols::IsotopeMethod& method = *pMethod_;

    //------------ add dummy data for debug ---------------
    //-----------------------------------------------------
    method.addFormula( adcontrols::IsotopeMethod::Formula( L"Xe", L"", 1, 1.0 ) );
    //-----------------------------------------------------

    QStandardItem * rootNode = model.invisibleRootItem();

    ui->treeView->setItemDelegate( pDelegate_.get() );

    rootNode->setColumnCount(2);
    model.setHeaderData( 0, Qt::Horizontal, "Isotope" );

    QStandardItem * item = StandardItemHelper::appendRow( rootNode, "Isotope" );

}

void
IsotopeForm::OnFinalClose()
{
}

void
IsotopeForm::getContents( adcontrols::ProcessMethod& pm )
{
    pm.appendMethod< adcontrols::IsotopeMethod >( *pMethod_ );
}