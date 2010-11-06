//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "peakidtableform.h"
#include "ui_peakidtableform.h"

using namespace qtwidgets;

PeakIDTableForm::PeakIDTableForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PeakIDTableForm)
{
    ui->setupUi(this);
}

PeakIDTableForm::~PeakIDTableForm()
{
    delete ui;
}
