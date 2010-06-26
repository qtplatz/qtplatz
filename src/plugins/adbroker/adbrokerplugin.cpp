//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "adbrokerplugin.h"
#include "adicontrols/massspectrum.h"

#include <QtCore/qplugin.h>

#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <QStringList>

using namespace ADBroker;
using namespace ADBroker::internal;

ADBrokerPlugin::~ADBrokerPlugin()
{
}

ADBrokerPlugin::ADBrokerPlugin()
{
}

bool
ADBrokerPlugin::initialize(const QStringList& arguments, QString* error_message)
{
  Q_UNUSED( arguments );
  Q_UNUSED( error_message );
  return true;
}


void
ADBrokerPlugin::extensionsInitialized()
{
}

Q_EXPORT_PLUGIN( ADBrokerPlugin )
