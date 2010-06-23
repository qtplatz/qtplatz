//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "fractions.h"
#include "import_sagraphics.h"

using namespace adil::ui;

Fractions::~Fractions()
{
  if ( pi_ )
    pi_->Release();
}

Fractions::Fractions( ISADPFractions * pi ) : pi_(pi)
{
  pi_->AddRef();
}

Fractions::Fractions( const Fractions& t )
{
   if ( t.pi_ )
     t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
     pi_->Release();
   pi_ = t.pi_;
}

