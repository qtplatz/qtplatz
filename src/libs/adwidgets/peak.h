// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef PEAK_H
#define PEAK_H

struct ISADPPeak;

namespace adil {
  namespace ui {

    class Peak  {
    public:
		~Peak();
		Peak( ISADPPeak * pi = 0 );
		Peak( const Peak& );
    private:
		ISADPPeak * pi_;
    };

  }
}

#endif // PEAK_H
