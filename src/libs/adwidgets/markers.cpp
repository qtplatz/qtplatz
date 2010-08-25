//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "markers.h"
#include "import_sagraphics.h"

using namespace adwidgets::ui;

Markers::~Markers()
{
  if ( pi_ )
    pi_->Release();
}

Markers::Markers( SAGRAPHICSLib::ISADPMarkers * pi ) : pi_(pi)
{
  pi_->AddRef();
}

Markers::Markers( const Markers& t )
{
   if ( t.pi_ )
     t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
     pi_->Release();
   pi_ = t.pi_;
}

