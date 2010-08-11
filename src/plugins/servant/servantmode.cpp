//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "servantmode.h"
#include <coreplugin/editormanager/editormanager.h>

using namespace servant;
using namespace servant::internal;

ServantMode::ServantMode(QObject *parent) :
    Core::BaseMode(parent)
{
    setName(tr("Servant"));
    setUniqueModeName( "Servant.Mode" );
    setIcon(QIcon(":/fancyactionbar/images/mode_Edit.png"));
    setPriority( 97 );
}

ServantMode::~ServantMode()
{
    Core::EditorManager::instance()->setParent(0);
}
