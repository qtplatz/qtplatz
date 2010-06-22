// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef PLOTREGION_H
#define PLOTREGION_H

struct ISADPPlotRegion;

namespace adil {
  
  namespace ui {
    
    class PlotRegion {
    public:
     ~PlotRegion();
      PlotRegion( ISADPPlotRegion * pi = 0 );
      PlotRegion( const PlotRegion& );

      private:
      ISADPPlotRegion * pi_;
    };
    
  }
}

#endif // PLOTREGION_H
