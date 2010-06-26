//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "annotations.h"
#include "import_sagraphics.h"

using namespace adil::ui;

Annotations::~Annotations()
{
  if ( pi_ )
    pi_->Release();
}

Annotations::Annotations( SAGRAPHICSLib::ISADPAnnotations * pi ) : pi_(pi)
{
  pi_->AddRef();
}

Annotations::Annotations( const Annotations& t )
{
   if ( t.pi_ )
     t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
     pi_->Release();
   pi_ = t.pi_;
}
