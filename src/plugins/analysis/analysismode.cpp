//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "analysismode.h"
#include <coreplugin/editormanager/editormanager.h>

using namespace Analysis;
using namespace Analysis::internal;

AnalysisMode::~AnalysisMode()
{
    Core::EditorManager::instance()->setParent(0);
}

AnalysisMode::AnalysisMode(QObject *parent) :
    Core::BaseMode(parent)
{
  setName(tr("Data Analysis"));
  setUniqueModeName( "Analysis.Mode" );
  setIcon(QIcon(":/fancyactionbar/images/mode_Debug.png"));
  setPriority( 97 );
}
