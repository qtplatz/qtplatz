//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////
#include "appplugin.h"
#include "outputwindow.h"
#include <QtCore/qplugin.h>
#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <QStringList>
#include <extensionsystem/pluginmanager.h>

using namespace App::internal;

AppPlugin::AppPlugin()
{
}

bool
AppPlugin::initialize(const QStringList& arguments, QString* error_message)
{
  Q_UNUSED( arguments );
  Q_UNUSED( error_message );
  
  ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
  if ( pm ) {
	QList<QObject *> objects = pm->allObjects();
  }
  
  addAutoReleasedObject( new OutputWindow );
  
  return true;
}


void
AppPlugin::extensionsInitialized()
{
}

void
AppPlugin::slotObjectAdded( QObject * obj )
{
  QString name = obj->objectName();
}

Q_EXPORT_PLUGIN( AppPlugin )
