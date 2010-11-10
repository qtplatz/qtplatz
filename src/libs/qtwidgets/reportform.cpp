//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "reportform.h"
#include "ui_reportform.h"
#include <adportable/configuration.h>
#include <QStandardItemModel>

using namespace qtwidgets;

ReportForm::ReportForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReportForm)
    , pModel_( new QStandardItemModel )
    , pConfig_( new adportable::Configuration )
{
    ui->setupUi(this);
}

ReportForm::~ReportForm()
{
    delete ui;
}

void
ReportForm::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;
}

void
ReportForm::OnInitialUpdate()
{
}

void
ReportForm::OnFinalClose()
{
}
