//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////

#include "acquireplugin.h"
#include "acquiremode.h"
#include <libqt/tracewidget.h>
#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>

using namespace Acquire;
using namespace Acquire::internal;

AcquirePlugin::~AcquirePlugin()
{
}

AcquirePlugin::AcquirePlugin()
{
}

bool
AcquirePlugin::initialize(const QStringList &arguments, QString *error_message)
{
  Core::ICore * core = Core::ICore::instance();
  if ( ! core )
    return false;

  QList<int> context;
  context.append( core->uniqueIDManager()->uniqueIdentifier( QLatin1String("Acquire.MainView") ) );

  AcquireMode * mode = new AcquireMode(this);
  if ( ! mode )
    return false;
  mode->setContext( context );

  libqt::TraceWidget * traceWidget = new libqt::TraceWidget;

  return true;
}
