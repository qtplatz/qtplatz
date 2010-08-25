//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "peaks.h"
#include "import_sagraphics.h"

using namespace adwidgets::ui;

Peaks::~Peaks()
{
  if ( pi_ )
    pi_->Release();
}

Peaks::Peaks( SAGRAPHICSLib::ISADPPeaks * pi ) : pi_(pi)
{
  pi_->AddRef();
}

Peaks::Peaks( const Peaks& t )
{
   if ( t.pi_ )
     t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
     pi_->Release();
   pi_ = t.pi_;
}
