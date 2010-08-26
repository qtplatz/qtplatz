//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include <comdef.h>
#include "color.h"
#include "import_sagraphics.h"

using namespace adwidgets::ui;

Color::~Color()
{
	if ( pi_ )
		pi_->Release();
}

Color::Color() : pi_(0)
{
}

void
Color::operator = ( const Color& t )
{
	if ( t.pi_ )
		t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
	if ( pi_ )
		pi_->Release();
	pi_ = t.pi_;
}

unsigned long /* OLE_COLOR */
Color::value() const
{
	return pi_->Value;
}

void
Color::value( const unsigned long /* OLE_COLOR*/ & color )
{
	pi_->Value = color;
}

void
Color::rgb( int r, int g, int b )
{
	pi_->Value = (r & 0xff) | ( g & 0xff ) << 8 | ( b & 0xff ) << 16;
}

int
Color::red() const
{
	return (pi_->Value & 0x00ff);
}

int
Color::green() const
{
	return (pi_->Value & 0x0000ff00) >> 8;
}

int
Color::blue() const
{
	return (pi_->Value & 0x00ff0000) >> 16;
}

