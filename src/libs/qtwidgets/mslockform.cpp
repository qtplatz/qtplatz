//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "mslockform.h"
#include "ui_mslockform.h"

using namespace qtwidgets;

MSLockForm::MSLockForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MSLockForm)
{
    ui->setupUi(this);
}

MSLockForm::~MSLockForm()
{
    delete ui;
}
