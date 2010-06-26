//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "titles.h"
#include "import_sagraphics.h"

using namespace adil::ui;

Titles::~Titles()
{
  if ( pi_ )
	  pi_->Release();
}

Titles::Titles( SAGRAPHICSLib::ISADPTitles * pi ) : pi_(pi)
{
	pi_->AddRef();
}

Titles::Titles( const Titles& t )
{
   if ( t.pi_ )
	   t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
	   pi_->Release();
   pi_ = t.pi_;
}