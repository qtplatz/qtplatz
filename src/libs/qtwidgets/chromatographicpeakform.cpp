//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "chromatographicpeakform.h"
#include "ui_chromatographicpeakform.h"

using namespace qtwidgets;

ChromatographicPeakForm::ChromatographicPeakForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChromatographicPeakForm)
{
    ui->setupUi(this);
}

ChromatographicPeakForm::~ChromatographicPeakForm()
{
    delete ui;
}
