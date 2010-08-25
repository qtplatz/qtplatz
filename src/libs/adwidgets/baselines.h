// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef BASELINES_H
#define BASELINES_H

namespace SAGRAPHICSLib {
struct ISADPBaselines;
}

namespace adwidgets {
  namespace ui {

    class Baselines  {
    public:
		~Baselines();
		Baselines( SAGRAPHICSLib::ISADPBaselines * pi = 0 );
		Baselines( const Baselines& );
    private:
        SAGRAPHICSLib::ISADPBaselines * pi_;
    };

  }
}


#endif // BASELINES_H
