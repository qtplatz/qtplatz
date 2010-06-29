// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <coreplugin/basemode.h>

namespace dataproc {
  namespace internal {

    class DataprocMode : public Core::BaseMode {
      Q_OBJECT
    public:
      ~DataprocMode();
      explicit DataprocMode( QObject * parent = 0 );

    signals:

    public slots:

    private:

    };
    ////////////////
  }
}


