// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ANALYSISMODE_H
#define ANALYSISMODE_H

#include <coreplugin/basemode.h>

namespace Analysis {
  namespace internal {

    class AnalysisMode : public Core::BaseMode {
      Q_OBJECT
    public:
      ~AnalysisMode();
      explicit AnalysisMode(QObject *parent = 0);

    signals:

    public slots:

    private:

    };
    ////////////////
  }
}

#endif // ANALYSISMODE_H
