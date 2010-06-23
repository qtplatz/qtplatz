// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef FILLEDRANGES_H
#define FILLEDRANGES_H

struct ISADPFilledRanges;

namespace adil {
  namespace ui {

    class FilledRanges {
    public:
      ~FilledRanges();
      FilledRanges( ISADPFilledRanges * pi = 0 );
      FilledRanges( const FilledRanges& );
	private:
      ISADPFilledRanges * pi_;
    };

  }
}

#endif // FILLEDRANGES_H
