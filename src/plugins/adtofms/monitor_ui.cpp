//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "monitor_ui.h"
#include <QtCore/qplugin.h>
#include "ui_form.h"

using namespace adtofms;

monitor_ui::monitor_ui(QWidget *parent) : IMonitor(parent)
                                        , ui(new Ui::Form)
{
    ui->setupUi(this);
}

monitor_ui::~monitor_ui()
{
	delete ui;
}

void
monitor_ui::OnInitialUpdate( const wchar_t * xml )
{
}

void
monitor_ui::OnFinalClose()
{
}

// Q_EXPORT_PLUGIN( monitor_ui )