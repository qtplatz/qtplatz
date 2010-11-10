//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "targetingform.h"
#include "ui_targetingform.h"
#include <adportable/configuration.h>
#include <QStandardItemModel>

using namespace qtwidgets;

TargetingForm::TargetingForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TargetingForm)
    , pModel_( new QStandardItemModel )
    , pConfig_( new adportable::Configuration )
{
    ui->setupUi(this);
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
}

void
TargetingForm::OnFinalClose()
{
}
