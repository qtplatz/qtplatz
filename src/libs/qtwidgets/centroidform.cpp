//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "centroidform.h"
#include "ui_centroidform.h"

using namespace qtwidgets;

CentroidForm::CentroidForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CentroidForm)
{
    ui->setupUi(this);
}

CentroidForm::~CentroidForm()
{
    delete ui;
}

void
CentroidForm::OnCreate( const adportable::Configuration& config )
{
    config_ = config;
}

void
CentroidForm::OnInitialUpdate()
{
}

void
CentroidForm::OnFinalClose()
{
}
