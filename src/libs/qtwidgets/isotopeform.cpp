//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "isotopeform.h"
#include "ui_isotopeform.h"
#include <adportable/configuration.h>
#include <QStandardItemModel>

using namespace qtwidgets;

IsotopeForm::IsotopeForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IsotopeForm)
    , pModel_( new QStandardItemModel )
    , pConfig_( new adportable::Configuration )
{
    ui->setupUi(this);
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
