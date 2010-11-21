//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "dataprocmode.h"
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/coreconstants.h>

using namespace dataproc;
using namespace dataproc::internal;

DataprocMode::~DataprocMode()
{
    Core::EditorManager::instance()->setParent(0);
}

DataprocMode::DataprocMode(QObject *parent) :
    Core::BaseMode(parent)
{
    setName(tr("Data processing"));
    setUniqueModeName( "Edit" );  // pretending to "Editor" for Core
    setIcon(QIcon(":/fancyactionbar/images/mode_Debug.png"));
    setPriority( 97 );
}
