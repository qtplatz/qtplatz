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

#include "peaks.h"
#include "peak.h"
#include "import_sagraphics.h"

using namespace adwidgets::ui;

Peaks::~Peaks()
{
    if ( pi_ )
        pi_->Release();
}

Peaks::Peaks( SAGRAPHICSLib::ISADPPeaks * pi ) : pi_(pi)
{
    pi_->AddRef();
}

Peaks::Peaks( const Peaks& t )
{
    if ( t.pi_ )
        t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
    if ( pi_ )
        pi_->Release();
    pi_ = t.pi_;
}

void
Peaks::clear()
{
    pi_->Clear();
}

Peak
Peaks::operator [] ( int idx )
{
    return Peak ( pi_->Item[ idx + 1 ] );
}

size_t
Peaks::size() const
{
    return pi_->Count;
}

bool
Peaks::visible() const
{
    return internal::variant_bool::to_native( pi_->Visible );
}

void
Peaks::visible( bool value )
{
    pi_->Visible = internal::variant_bool::to_variant( value );
}

Peak
Peaks::add()
{
    return Peak( pi_->Add() );
}

