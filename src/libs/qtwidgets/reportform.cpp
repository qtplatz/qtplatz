//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "reportform.h"
#include "ui_reportform.h"

using namespace qtwidgets;

ReportForm::ReportForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReportForm)
{
    ui->setupUi(this);
}

ReportForm::~ReportForm()
{
    delete ui;
}
