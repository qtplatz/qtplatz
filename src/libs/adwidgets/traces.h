// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef TRACES_H
#define TRACES_H

struct ISADPTraces;

namespace adil {
  namespace ui {

    class Traces {
    public:
      ~Traces();
      Traces( ISADPTraces * pi = 0 );
      Traces( const Traces& );

    private:
      ISADPTraces * pi_;
    };
  }
}

#endif // TRACES_H
