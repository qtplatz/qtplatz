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