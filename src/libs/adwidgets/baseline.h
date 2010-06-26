// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef BASELINE_H
#define BASELINE_H

namespace SAGRAPHICSLib {
struct ISADPBaseline;
}

namespace adil {
  namespace ui {

    class Baseline  {
    public:
		~Baseline();
		Baseline( SAGRAPHICSLib::ISADPBaseline * pi = 0 );
		Baseline( const Baseline& );
    private:
        SAGRAPHICSLib::ISADPBaseline * pi_;
    };

  }
}

#endif // BASELINE_H
