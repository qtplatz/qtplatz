#include "acquiremode.h"
#include <coreplugin/editormanager/editormanager.h>

using namespace Acquire;
using namespace Acquire::internal;

AcquireMode::~AcquireMode()
{
    Core::EditorManager::instance()->setParent(0);
}

AcquireMode::AcquireMode(QObject *parent) :
    Core::BaseMode(parent)
{
    setName(tr("Acquire"));
    setUniqueModeName( "Acquire.Mode" );
    setIcon(QIcon(":/fancyactionbar/images/mode_Debug.png"));
    setPriority( 99 );
}
