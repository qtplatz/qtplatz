// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef FILLEDRANGES_H
#define FILLEDRANGES_H

namespace SAGRAPHICSLib {
struct ISADPFilledRanges;
}

namespace adwidgets {
  namespace ui {

    class FilledRanges {
    public:
      ~FilledRanges();
      FilledRanges( SAGRAPHICSLib::ISADPFilledRanges * pi = 0 );
      FilledRanges( const FilledRanges& );
	private:
        SAGRAPHICSLib::ISADPFilledRanges * pi_;
    };

  }
}

#endif // FILLEDRANGES_H
