// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ANNOTATIONS_H
#define ANNOTATIONS_H

struct ISADPAnnotations;

namespace adil {
  namespace ui {

    class Annotations {
    public:
      ~Annotations();
      Annotations( ISADPAnnotations * pi = 0 );
      Annotations( const Annotations& );
      
    private:
      ISADPAnnotations * pi_;
    };

  }
}

#endif // ANNOTATIONS_H
