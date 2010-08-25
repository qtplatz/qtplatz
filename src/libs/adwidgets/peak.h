// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef PEAK_H
#define PEAK_H

namespace SAGRAPHICSLib {
struct ISADPPeak;
}

namespace adwidgets {
  namespace ui {

    class Peak  {
    public:
		~Peak();
		Peak( SAGRAPHICSLib::ISADPPeak * pi = 0 );
		Peak( const Peak& );
    private:
        SAGRAPHICSLib::ISADPPeak * pi_;
    };

  }
}

#endif // PEAK_H
