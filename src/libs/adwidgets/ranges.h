// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef RANGES_H
#define RANGES_H

struct ISADPRanges;

namespace adil {
  namespace ui {

    class Ranges  {
    public:
		~Ranges();
		Ranges( ISADPRanges * pi = 0 );
		Ranges( const Ranges& );
    private:
		ISADPRanges * pi_;
    };

  }
}

#endif // RANGES_H
