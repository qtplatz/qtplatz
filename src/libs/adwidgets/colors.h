// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef COLORS_H
#define COLORS_H

namespace SAGRAPHICSLib {
    struct ISADPColors;
}

namespace adwidgets {
  namespace ui {

    class Colors  {
    public:
      ~Colors();
      Colors( SAGRAPHICSLib::ISADPColors * pi = 0 );
      Colors( const Colors& );
      void operator = ( const Colors& );
      size_t size() const;

    private:
        SAGRAPHICSLib::ISADPColors * pi_;
    };
  }
}

#endif // COLORS_H
