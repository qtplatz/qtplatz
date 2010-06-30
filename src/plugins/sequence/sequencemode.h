// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef SEQUENCEMODE_H
#define SEQUENCEMODE_H

#include <coreplugin/basemode.h>

namespace sequence {
  namespace internal {
    class SequenceMode : public Core::BaseMode {
      Q_OBJECT
    public:
      explicit SequenceMode(QObject *parent = 0);

    signals:

    public slots:

    private:

    };

  }
}

#endif // SEQUENCEMODE_H
