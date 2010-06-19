// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef SEQUENCEPLUGIN_H
#define SEQUENCEPLUGIN_H

#include <extensionsystem/iplugin.h>

namespace Sequence {
  namespace internal {

    class SequencePlugin : public ExtensionSystem::IPlugin {
      Q_OBJECT;
    public:
      ~SequencePlugin();
      explicit SequencePlugin();

      bool initialize(const QStringList& arguments, QString* error_message);

    signals:

    public slots:

    };
    //------
  }
}

#endif // SEQUENCEPLUGIN_H
