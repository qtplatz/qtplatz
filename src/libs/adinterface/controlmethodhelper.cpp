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

#include "controlmethodhelper.hpp"

using namespace adinterface;

ControlMethodHelper::ControlMethodHelper()
{
}

ControlMethodHelper::ControlMethodHelper( const ControlMethodHelper& t ) : method_(t.method_)
{
}

ControlMethodHelper::ControlMethodHelper( const ControlMethod::Method& m ) : method_(m)
{
}

const wchar_t *
ControlMethodHelper::subject() const 
{
    return method_.subject.in();
}

void
ControlMethodHelper::subject( const std::wstring& text )
{
    method_.subject = CORBA::wstring_dup( text.c_str() );
}

const wchar_t *
ControlMethodHelper::description() const 
{
    return method_.description.in();
}

void
ControlMethodHelper::description( const std::wstring& text )
{
    method_.description = CORBA::wstring_dup( text.c_str() );
}
