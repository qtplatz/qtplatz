//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "elementalcompositionform.h"
#include "ui_elementalcompositionform.h"

#include <adportable/configuration.h>
#include <QStandardItemModel>

using namespace qtwidgets;

ElementalCompositionForm::ElementalCompositionForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ElementalCompositionForm)
    , pModel_( new QStandardItemModel )
    , pConfig_( new adportable::Configuration )
{
    ui->setupUi(this);
}

ElementalCompositionForm::~ElementalCompositionForm()
{
    delete ui;
}

void
ElementalCompositionForm::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;
}

void
ElementalCompositionForm::OnInitialUpdate()
{
}

void
ElementalCompositionForm::OnFinalClose()
{
}
