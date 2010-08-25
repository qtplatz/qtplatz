//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "colors.h"
#include "import_sagraphics.h"

using namespace adwidgets::ui;

Colors::~Colors()
{
  if ( pi_ )
    pi_->Release();
}

Colors::Colors( SAGRAPHICSLib::ISADPColors * pi ) : pi_(pi)
{
  pi_->AddRef();
}

Colors::Colors( const Colors& t )
{
   if ( t.pi_ )
     t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
     pi_->Release();
   pi_ = t.pi_;
}

void
Colors::operator = ( const Colors& t )
{
   if ( t.pi_ )
     t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
     pi_->Release();
   pi_ = t.pi_;
}

size_t
Colors::size() const
{
	return pi_->Count;
}