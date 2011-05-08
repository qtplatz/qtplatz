// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "qstring.hpp"
#include <adportable/utf.hpp>

using namespace qtwrapper;

qstring::qstring( const std::wstring& t ) 
: impl_( QString::fromUtf16( reinterpret_cast<const UTF16 *>( t.c_str() ) ) )
{
}

QString
qstring::copy( const std::wstring& t )
{
    QString res( QString::fromUtf16( reinterpret_cast<const UTF16 *>( t.c_str() ) ) );
    return res;
}


wstring::wstring( const QString& t ) : impl_( wstring::copy(t) )
{
}

std::wstring
wstring::copy( const QString& t )
{
#if defined WIN32
    return std::wstring( reinterpret_cast<const wchar_t *>( t.utf16() ) );
#else
    return t.toWStdString();
#endif
}

