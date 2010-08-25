// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef PLOTREGION_H
#define PLOTREGION_H

namespace SAGRAPHICSLib {
struct ISADPPlotRegion;
}

namespace adwidgets {
  
  namespace ui {
    
    class PlotRegion {
    public:
     ~PlotRegion();
      PlotRegion( SAGRAPHICSLib::ISADPPlotRegion * pi = 0 );
      PlotRegion( const PlotRegion& );

      private:
          SAGRAPHICSLib::ISADPPlotRegion * pi_;
    };
    
  }
}

#endif // PLOTREGION_H
