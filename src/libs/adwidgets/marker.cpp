//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "marker.h"
#include "import_sagraphics.h"
using namespace adwidgets::ui;

Marker::~Marker()
{
  if ( pi_ )
    pi_->Release();
}

Marker::Marker( SAGRAPHICSLib::ISADPMarker * pi ) : pi_(pi)
{
  pi_->AddRef();
}

Marker::Marker( const Marker& t )
{
   if ( t.pi_ )
     t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
     pi_->Release();
   pi_ = t.pi_;
}

