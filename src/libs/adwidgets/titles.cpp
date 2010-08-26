//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "titles.h"
#include "title.h"
#include "import_sagraphics.h"

using namespace adwidgets::ui;

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

size_t
Titles::count() const
{
	return pi_->Count;
}

Title
Titles::item( long Index )
{
	return Title( pi_->GetItem( Index ) );
}

Title
Titles::operator [] ( int Index )
{
	return Title( pi_->GetItem( Index + 1) );
}

void
Titles::visible( bool visible )
{
	pi_->Visible = internal::variant_bool::to_variant( visible );
}

bool
Titles::visible() const
{
	return internal::variant_bool::to_native( pi_->Visible );
}