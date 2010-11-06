//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "sequencesform.h"
#include "ui_sequencesform.h"
#include "sequencesmodel.h"

using namespace qtwidgets;

SequencesForm::SequencesForm(QWidget *parent) : QWidget(parent)
                                              , ui(new Ui::SequencesForm)
                                              , pModel_( new qtwidgets::SequencesModel )
{
    ui->setupUi(this);
    ui->treeView->setModel( pModel_ );
}

SequencesForm::~SequencesForm()
{
    delete ui;
    delete pModel_;
}

void
SequencesForm::OnCreate( const adportable::Configuration& config )
{
    config_ = config;
}

void
SequencesForm::OnInitialUpdate()
{
}

void
SequencesForm::OnFinalClose()
{
}
