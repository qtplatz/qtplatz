//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "axis.h"

#include "import_sagraphics.h"

using namespace adil::ui;

Axis::~Axis()
{
  if ( pi_ )
	  pi_->Release();
}

Axis::Axis( ISADPAxis * pi ) : pi_(pi)
{
	pi_->AddRef();
}

Axis::Axis( const Axis& axis )
{
   if ( axis.pi_ )
	   axis.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
	   pi_->Release();
   pi_ = axis.pi_;
}