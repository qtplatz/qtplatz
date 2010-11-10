//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "chromatographicpeakform.h"
#include "ui_chromatographicpeakform.h"
#include <adportable/configuration.h>
#include <QStandardItemModel>

using namespace qtwidgets;

ChromatographicPeakForm::ChromatographicPeakForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChromatographicPeakForm)
    , pModel_( new QStandardItemModel )
    , pConfig_( new adportable::Configuration )

{
    ui->setupUi(this);
}

ChromatographicPeakForm::~ChromatographicPeakForm()
{
    delete ui;
}

void
ChromatographicPeakForm::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;
}

void
ChromatographicPeakForm::OnInitialUpdate()
{
}

void
ChromatographicPeakForm::OnFinalClose()
{
}
