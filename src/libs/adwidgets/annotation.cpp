//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "annotation.h"
#include "import_sagraphics.h"

using namespace adil::ui;

Annotation::~Annotation()
{
  if ( pi_ )
    pi_->Release();
}

Annotation::Annotation( ISADPAnnotation * pi ) : pi_(pi)
{
  pi_->AddRef();
}

Annotation::Annotation( const Annotation& t )
{
   if ( t.pi_ )
     t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
     pi_->Release();
   pi_ = t.pi_;
}

