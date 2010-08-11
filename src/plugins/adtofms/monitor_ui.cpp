//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "monitor_ui.h"
#include <QtCore/qplugin.h>

using namespace adtofms;

monitor_ui::monitor_ui(QWidget *parent) : IMonitor(parent)
{
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