//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "isotopeform.h"
#include "ui_isotopeform.h"

using namespace qtwidgets;

IsotopeForm::IsotopeForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IsotopeForm)
{
    ui->setupUi(this);
}

IsotopeForm::~IsotopeForm()
{
    delete ui;
}
