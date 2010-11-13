//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "mscalibrationform.h"
#include "ui_mscalibrationform.h"
#include <adcontrols/mscalibratemethod.h>
#include <adportable/configuration.h>
#include <QStandardItemModel>

using namespace qtwidgets;

MSCalibrationForm::MSCalibrationForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MSCalibrationForm)
    , pModel_( new QStandardItemModel )
    , pConfig_( new adportable::Configuration )
    , pMethod_( new adcontrols::MSCalibrateMethod ) 
{
    ui->setupUi(this);
}

MSCalibrationForm::~MSCalibrationForm()
{
    delete ui;
}

void
MSCalibrationForm::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;
}

void
MSCalibrationForm::OnInitialUpdate()
{
}

void
MSCalibrationForm::OnFinalClose()
{
}
