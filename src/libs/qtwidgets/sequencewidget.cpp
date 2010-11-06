
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "sequencewidget.h"
#include "ui_sequencewidget.h"
#include "sequencemodel.h"

using namespace qtwidgets;

SequenceWidget::SequenceWidget(QWidget *parent) : QWidget(parent)
                                                , ui(new Ui::SequenceWidget)
                                                , pModel_( new SequenceModel )
{
    ui->setupUi(this);
    ui->treeView->setModel( pModel_ );
}

SequenceWidget::~SequenceWidget()
{
    delete ui;
    delete pModel_;
}
