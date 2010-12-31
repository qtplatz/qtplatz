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
