// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef LEGEND_H
#define LEGEND_H

namespace SAGRAPHICSLib {
struct ISADPLegend;
}

namespace adwidgets {
  namespace ui {
    class Legend	  {
    public:
      ~Legend();
      Legend( SAGRAPHICSLib::ISADPLegend * pi = 0 );
      Legend( const Legend& );
	private:
      SAGRAPHICSLib::ISADPLegend * pi_;
    };
  }
}
#endif // LEGEND_H
