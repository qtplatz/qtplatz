//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "legend.h"
#include "import_sagraphics.h"

using namespace adwidgets::ui;

Legend::~Legend()
{
  if ( pi_ )
    pi_->Release();
}

Legend::Legend( SAGRAPHICSLib::ISADPLegend * pi ) : pi_(pi)
{
  pi_->AddRef();
}

Legend::Legend( const Legend& t )
{
   if ( t.pi_ )
     t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
     pi_->Release();
   pi_ = t.pi_;
}
