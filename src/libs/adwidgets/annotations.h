// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ANNOTATIONS_H
#define ANNOTATIONS_H

namespace SAGRAPHICSLib {
struct ISADPAnnotations;
}

namespace adwidgets {
  namespace ui {

    class Annotations {
    public:
      ~Annotations();
      Annotations( SAGRAPHICSLib::ISADPAnnotations * pi = 0 );
      Annotations( const Annotations& );
      
    private:
        SAGRAPHICSLib::ISADPAnnotations * pi_;
    };

  }
}

#endif // ANNOTATIONS_H
