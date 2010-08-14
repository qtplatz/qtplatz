//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "logwidget.h"
#include "ui_logwidget.h"

using namespace qtwidgets;

LogWidget::LogWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LogWidget)
{
    ui->setupUi(this);
}

LogWidget::~LogWidget()
{
    delete ui;
}

void
LogWidget::OnCreate( const adportable::Configuration& config )
{
    config_ = config;
}

void
LogWidget::OnInitialUpdate()
{
}

void
LogWidget::OnFinalClose()
{
}
