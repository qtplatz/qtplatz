//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "mslockform.h"
#include "ui_mslockform.h"
#include <adportable/configuration.h>
#include <QStandardItemModel>

using namespace qtwidgets;

MSLockForm::MSLockForm(QWidget *parent) : QWidget(parent)
                                        , ui(new Ui::MSLockForm)
                                        , pConfig_( new adportable::Configuration )
{
    ui->setupUi(this);
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
}

void
MSLockForm::OnFinalClose()
{
}
