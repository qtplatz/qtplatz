//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "elementalcompositionform.h"
#include "ui_elementalcompositionform.h"
#include "elementalcompositiondelegate.h"
#include "standarditemhelper.h"
#include <adportable/configuration.h>
#include <QStandardItemModel>

using namespace qtwidgets;

ElementalCompositionForm::ElementalCompositionForm(QWidget *parent) :
    QWidget(parent)
    , ui(new Ui::ElementalCompositionForm)
    , pModel_( new QStandardItemModel )
    , pConfig_( new adportable::Configuration )
    , pDelegate_( new ElementalCompositionDelegate )
{
    ui->setupUi(this);
}

ElementalCompositionForm::~ElementalCompositionForm()
{
    delete ui;
}

void
ElementalCompositionForm::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;
}

void
ElementalCompositionForm::OnInitialUpdate()
{
    QStandardItemModel& model = *pModel_;
    QStandardItem * rootNode = model.invisibleRootItem();
    
    ui->treeView->setItemDelegate( pDelegate_.get() );
    
    rootNode->setColumnCount(2);
    model.setHeaderData( 0, Qt::Horizontal, "Elemental Composition" );
    
    StandardItemHelper::appendRow( rootNode, "Mass", 0.00 );
}

void
ElementalCompositionForm::OnFinalClose()
{
}
