//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "targetingform.h"
#include "ui_targetingform.h"

using namespace qtwidgets;

TargetingForm::TargetingForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TargetingForm)
{
    ui->setupUi(this);
}

TargetingForm::~TargetingForm()
{
    delete ui;
}
