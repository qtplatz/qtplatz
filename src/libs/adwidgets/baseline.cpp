//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "baseline.h"
#include "import_sagraphics.h"

using namespace adil::ui;

Baseline::~Baseline()
{
  if ( pi_ )
    pi_->Release();
}

Baseline::Baseline( SAGRAPHICSLib::ISADPBaseline * pi ) : pi_(pi)
{
  pi_->AddRef();
}

Baseline::Baseline( const Baseline& t )
{
   if ( t.pi_ )
     t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
     pi_->Release();
   pi_ = t.pi_;
}

