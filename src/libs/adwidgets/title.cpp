//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "title.h"
#include "font.h"

using namespace adwidgets::ui;

#include "import_sagraphics.h"

using namespace adwidgets::ui;

Title::~Title()
{
	if ( pi_ )
		pi_->Release();
}

Title::Title( SAGRAPHICSLib::ISADPTitle * pi ) : pi_(pi)
{
	if ( pi_ )
		pi_->AddRef();
}

void
Title::operator = ( const Title& t )
{
	if ( t.pi_ )
		t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
	if ( pi_ )
		pi_->Release();
	pi_ = t.pi_;
}

std::wstring
Title::text() const
{
	std::wstring str = pi_->Text;
	return str;
}

void
Title::text( const std::wstring& str )
{
	pi_->Text = str.c_str();
}

bool
Title::visible() const
{
	return internal::variant_bool::to_native( pi_->Visible );
}

void
Title::visible( bool visible )
{
	pi_->Visible = internal::variant_bool::to_variant( visible );
}

unsigned long
Title::color() const
{
	return pi_->Color;
}

void
Title::color( unsigned long c )
{
	pi_->Color = c;
}

Font
Title::font()
{
	return Font( pi_->Font );
}
