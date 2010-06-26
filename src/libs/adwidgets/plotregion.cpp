//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "plotregion.h"
#include "import_sagraphics.h"

using namespace adil;
using namespace adil::ui;

PlotRegion::~PlotRegion()
{
  if ( pi_ )
    pi_->Release();
}

PlotRegion::PlotRegion( SAGRAPHICSLib::ISADPPlotRegion * pi ) : pi_(pi)
{
  pi_->AddRef();
}

PlotRegion::PlotRegion( const PlotRegion& t )
{
   if ( t.pi_ )
     t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
     pi_->Release();
   pi_ = t.pi_;
}
