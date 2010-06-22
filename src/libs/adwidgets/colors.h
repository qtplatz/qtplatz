// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef COLORS_H
#define COLORS_H

struct ISADPColors;

namespace adil {
  namespace ui {

    class Colors  {
    public:
      ~Colors();
      Colors( ISADPColors * pi = 0 );
      Colors( const Colors& );

    private:
      ISADPColors * pi_;
    };
  }
}

#endif // COLORS_H
