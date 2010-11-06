//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "elementalcompositionform.h"
#include "ui_elementalcompositionform.h"

using namespace qtwidgets;

ElementalCompositionForm::ElementalCompositionForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ElementalCompositionForm)
{
    ui->setupUi(this);
}

ElementalCompositionForm::~ElementalCompositionForm()
{
    delete ui;
}
