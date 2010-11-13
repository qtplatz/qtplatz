//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "isotopeform.h"
#include "ui_isotopeform.h"
#include <adcontrols/isotopemethod.h>
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
}

void
IsotopeForm::OnFinalClose()
{
}
