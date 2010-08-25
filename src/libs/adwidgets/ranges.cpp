//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "ranges.h"
#include "import_sagraphics.h"

using namespace adwidgets::ui;

Ranges::~Ranges()
{
  if ( pi_ )
    pi_->Release();
}

Ranges::Ranges( SAGRAPHICSLib::ISADPRanges * pi ) : pi_(pi)
{
  pi_->AddRef();
}

Ranges::Ranges( const Ranges& t )
{
   if ( t.pi_ )
     t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
     pi_->Release();
   pi_ = t.pi_;
}


