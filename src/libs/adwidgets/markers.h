// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef MARKERS_H
#define MARKERS_H

struct ISADPMarkers;

namespace adil {
  namespace ui {

    class Markers  {
    public:
		~Markers();
		Markers( ISADPMarkers * pi = 0 );
		Markers( const Markers& );
    private:
		ISADPMarkers * pi_;
    };

  }
}

#endif // MARKERS_H
