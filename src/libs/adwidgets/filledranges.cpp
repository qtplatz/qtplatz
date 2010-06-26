//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "filledranges.h"
#include "import_sagraphics.h"

using namespace adil::ui;

FilledRanges::~FilledRanges()
{
  if ( pi_ )
    pi_->Release();
}

FilledRanges::FilledRanges( SAGRAPHICSLib::ISADPFilledRanges * pi ) : pi_(pi)
{
  pi_->AddRef();
}

FilledRanges::FilledRanges( const FilledRanges& t )
{
   if ( t.pi_ )
     t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
     pi_->Release();
   pi_ = t.pi_;
}

