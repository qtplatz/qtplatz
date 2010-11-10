
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "sequencewidget.h"
#include "ui_sequencewidget.h"
#include <adportable/configuration.h>
#include <QStandardItemModel>

using namespace qtwidgets;

SequenceWidget::SequenceWidget(QWidget *parent) : QWidget(parent)
                                                , ui(new Ui::SequenceWidget)
                                                , pModel_( new QStandardItemModel )
                                                , pConfig_( new adportable::Configuration )
{
    ui->setupUi(this);
    ui->treeView->setModel( pModel_.get() );
}

SequenceWidget::~SequenceWidget()
{
    delete ui;
}

void
SequenceWidget::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;
}

void
SequenceWidget::OnInitialUpdate()
{
}

void
SequenceWidget::OnFinalClose()
{
}
