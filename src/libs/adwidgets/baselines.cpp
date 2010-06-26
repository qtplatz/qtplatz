//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "baselines.h"
#include "import_sagraphics.h"

using namespace adil::ui;

Baselines::~Baselines()
{
  if ( pi_ )
    pi_->Release();
}

Baselines::Baselines( SAGRAPHICSLib::ISADPBaselines * pi ) : pi_(pi)
{
  pi_->AddRef();
}

Baselines::Baselines( const Baselines& t )
{
   if ( t.pi_ )
     t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
     pi_->Release();
   pi_ = t.pi_;
}

