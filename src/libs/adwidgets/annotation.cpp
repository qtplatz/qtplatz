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

#include "annotation.h"
#include "import_sagraphics.h"

using namespace adwidgets::ui;

Annotation::~Annotation()
{
  if ( pi_ )
    pi_->Release();
}

Annotation::Annotation( SAGRAPHICSLib::ISADPAnnotation * pi ) : pi_(pi)
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

//
std::wstring 
Annotation::value() const
{
    CComBSTR bstr( pi_->Value );
    return static_cast<wchar_t *>( bstr );
}

template<> void 
Annotation::value( const std::wstring value )
{
    pi_->Value = _variant_t( value.c_str() );
}

template<> void 
Annotation::value( double value )
{
    pi_->Value = value;
}

template<> void 
Annotation::value( long value )
{
    pi_->Value = value;
}

template<> void 
Annotation::value( unsigned long value )
{
    pi_->Value = value;
}

double 
Annotation::x() const
{
    return pi_->x;
}

void 
Annotation::x( double value )
{
    pi_->x = value;
}

double 
Annotation::y() const
{
    return pi_->y;
}

void 
Annotation::y( double value )
{
    pi_->y = value;
}

long 
Annotation::priority() const
{
    return pi_->Priority;
}

void 
Annotation::priority( long value )
{
    pi_->Priority = value;
}

short 
Annotation::colorIndex() const
{
    return pi_->ColorIndex;
}

void 
Annotation::colorIndex( short value )
{
    pi_->ColorIndex = value;
}


