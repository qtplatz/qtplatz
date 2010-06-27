// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#pragma once

#include "appplugin_global.h"
#include <extensionsystem/iplugin.h>

namespace App {
  namespace internal {

    class APPPLUGIN_EXPORT AppPlugin : public ExtensionSystem::IPlugin {
      Q_OBJECT

    public:
      
      AppPlugin();
      bool initialize(const QStringList &arguments, QString *error_message);
      void extensionsInitialized();
      
    signals:

    public slots:
      void slotObjectAdded( QObject * obj );
    };
	
  }
}

