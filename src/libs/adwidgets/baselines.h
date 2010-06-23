// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef BASELINES_H
#define BASELINES_H

struct ISADPBaselines;

namespace adil {
  namespace ui {

    class Baselines  {
    public:
		~Baselines();
		Baselines( ISADPBaselines * pi = 0 );
		Baselines( const Baselines& );
    private:
		ISADPBaselines * pi_;
    };

  }
}


#endif // BASELINES_H
