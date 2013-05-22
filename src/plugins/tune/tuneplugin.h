// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Copyright (C) MS-Cheminformatics LLC Project
//////////////////////////////////////////////

#ifndef TUNEPLUGIN_H
#define TUNEPLUGIN_H

#include <extensionsystem/iplugin.h>

namespace Tune {
  namespace internal {

    class TunePlugin : public ExtensionSystem::IPlugin {
      Q_OBJECT
	;
    public:
      ~TunePlugin();
      explicit TunePlugin();
      bool initialize(const QStringList& arguments, QString * error_message);

    signals:

    public slots:

    };
    //----------
  }
}

#endif // TUNEPLUGIN_H
