// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////

#ifndef ACQUIREPLUGIN_H
#define ACQUIREPLUGIN_H

#include <extensionsystem/iplugin.h>

namespace Acquire {
  namespace internal {

    class AcquireUIManager;

    class AcquirePlugin : public ExtensionSystem::IPlugin {
      Q_OBJECT
    public:
      ~AcquirePlugin();
      AcquirePlugin();
      bool initialize(const QStringList &arguments, QString *error_message);
      void extensionsInitialized();
    private slots:

    private:
      AcquireUIManager * manager_;
    };
  }
}

#endif // ACQUIREPLUGIN_H
