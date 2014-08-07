//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

#include "servantmode.hpp"
#include <coreplugin/editormanager/editormanager.h>

using namespace servant;
using namespace servant::internal;

ServantMode::ServantMode(QObject *parent) : Core::IMode(parent)
{
    setDisplayName( tr( "Servant" ) );
    // setUniqueModeName( "Servant.Mode" );
    setIcon(QIcon(":/fancyactionbar/images/mode_Edit.png"));
    setPriority( 999 );
}

ServantMode::~ServantMode()
{
    Core::EditorManager::instance()->setParent(0);
}
