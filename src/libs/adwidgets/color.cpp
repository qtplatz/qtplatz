/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/
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

