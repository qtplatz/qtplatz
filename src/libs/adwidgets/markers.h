// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef MARKERS_H
#define MARKERS_H

namespace SAGRAPHICSLib {
struct ISADPMarkers;
}

namespace adwidgets {
  namespace ui {

    class Markers  {
    public:
		~Markers();
		Markers( SAGRAPHICSLib::ISADPMarkers * pi = 0 );
		Markers( const Markers& );
    private:
        SAGRAPHICSLib::ISADPMarkers * pi_;
    };

  }
}

#endif // MARKERS_H
