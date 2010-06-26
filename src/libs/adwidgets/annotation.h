// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef ANNOTATION_H
#define ANNOTATION_H

namespace SAGRAPHICSLib {
    struct ISADPAnnotation;
}

namespace adil {
  namespace ui {

    class Annotation {
    public:
      ~Annotation();
      Annotation( SAGRAPHICSLib::ISADPAnnotation * pi = 0 );
      Annotation( const Annotation& );
    private:
        SAGRAPHICSLib::ISADPAnnotation * pi_;
    };

  }
}

#endif // ANNOTATION_H
