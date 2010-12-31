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

#include "baseline.h"
#include "import_sagraphics.h"

using namespace adwidgets::ui;

Baseline::~Baseline()
{
    if ( pi_ )
        pi_->Release();
}

Baseline::Baseline( SAGRAPHICSLib::ISADPBaseline * pi ) : pi_(pi)
{
    pi_->AddRef();
}

Baseline::Baseline( const Baseline& t )
{
    if ( t.pi_ )
        t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
    if ( pi_ )
        pi_->Release();
    pi_ = t.pi_;
}

////

bool 
Baseline::visible() const
{
    return internal::variant_bool::to_variant( pi_->Visible );
}

void 
Baseline::visible( bool value )
{
    pi_->Visible = internal::variant_bool::to_variant( value );
}

short 
Baseline::colorIndex() const
{
    return pi_->ColorIndex;
}

void 
Baseline::colorIndex( short value )
{
    pi_->ColorIndex = value;
}

double 
Baseline::startX() const
{
    return pi_->StartX;
}

void 
Baseline::startX( double value )
{
    pi_->StartX = value;
}

double 
Baseline::startY() const
{
    return pi_->StartY;
}

void 
Baseline:: startY( double value )
{
    pi_->StartY = value;
}

double 
Baseline::endX() const
{
    return pi_->EndX;
}

void 
Baseline::endX( double value )
{
    pi_->EndX = value;
}

double 
Baseline::endY() const
{
    return pi_->EndY;
}

void 
Baseline::endY( double value )
{
    pi_->EndY = value;
}

bool 
Baseline::marked() const
{
    return internal::variant_bool::to_native( pi_->Marked );
}

void 
Baseline::marked( bool value )
{
    pi_->Marked = internal::variant_bool::to_variant( value );
}


