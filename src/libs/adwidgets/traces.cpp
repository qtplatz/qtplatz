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

#include "traces.h"
#include "trace.h"
#include "import_sagraphics.h"

using namespace adwidgets::ui;

Traces::~Traces()
{
    if ( pi_ )
        pi_->Release();
}

Traces::Traces( SAGRAPHICSLib::ISADPTraces * pi ) : pi_(pi)
{
    pi_->AddRef();
}

Traces::Traces( const Traces& t )
{
    if ( t.pi_ )
        t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
    if ( pi_ )
        pi_->Release();
    pi_ = t.pi_;
}

void
Traces::operator = ( const Traces& t )
{
    if ( t.pi_ )
        t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
    if ( pi_ )
        pi_->Release();
    pi_ = t.pi_;
}

Trace
Traces::operator [] (long idx)
{
    return item( idx + 1 );  // 0 origin
}

Trace
Traces::item(long Index) // 1 origin
{
    CComPtr<SAGRAPHICSLib::ISADPTrace> p;
    pi_->get_Item( Index, &p );
    return Trace( p );
}

size_t
Traces::size() const
{
  long nSize = 0;
  if ( pi_->get_Count(&nSize) == S_OK )
    return size_t(nSize);
  return 0;
}

Trace
Traces::add()
{
  CComPtr<SAGRAPHICSLib::ISADPTrace> p;
  return Trace( pi_->Add() );
}

void
Traces::remove(long Index)
{
  pi_->Remove( Index );
}

void
Traces::clear()
{
  pi_->Clear();
}

bool
Traces::visible() const
{
	VARIANT_BOOL v;
	pi_->get_Visible(&v);
	return internal::variant_bool::to_native(v);
}

void
Traces::visible(bool newValue)
{
	HRESULT hr = pi_->put_Visible( internal::variant_bool::to_variant(newValue) );
    (void)hr;
}


