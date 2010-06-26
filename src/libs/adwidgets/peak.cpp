// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "peak.h"
#include "import_sagraphics.h"

using namespace adil::ui;

Peak::~Peak()
{
  if ( pi_ )
    pi_->Release();
}

Peak::Peak( SAGRAPHICSLib::ISADPPeak * pi ) : pi_(pi)
{
  pi_->AddRef();
}

Peak::Peak( const Peak& t )
{
   if ( t.pi_ )
     t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
     pi_->Release();
   pi_ = t.pi_;
}

