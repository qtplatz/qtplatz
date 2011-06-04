//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "peakidtableform.hpp"
#include "ui_peakidtableform.h"
#include <adportable/configuration.hpp>
#include <QStandardItemModel>

using namespace qtwidgets;

PeakIDTableForm::PeakIDTableForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PeakIDTableForm)
    , pConfig_( new adportable::Configuration )
{
    ui->setupUi(this);
}

PeakIDTableForm::~PeakIDTableForm()
{
    delete ui;
}

void
PeakIDTableForm::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;
}

void
PeakIDTableForm::OnInitialUpdate()
{
}

void
PeakIDTableForm::OnFinalClose()
{
}

QSize
PeakIDTableForm::sizeHint() const
{
    return QSize( 300, 250 );
}
