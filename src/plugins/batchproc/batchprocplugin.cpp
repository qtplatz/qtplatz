#include "batchprocplugin.h"

using namespace Batchproc;
using namespace Batchproc::internal;

BatchprocPlugin::~BatchprocPlugin()
{
}

BatchprocPlugin::BatchprocPlugin()
{
}

bool
BatchprocPlugin::initialize(const QStringList&, QString* )
{
    return false;
}
