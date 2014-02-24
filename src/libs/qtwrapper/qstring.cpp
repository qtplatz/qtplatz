// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

qstring::qstring( const std::wstring& t ) : impl_( copy( t ) )
{
}

QString
qstring::copy( const std::wstring& t )
{
#if defined WIN32
    QString res( QString::fromUtf16( reinterpret_cast<const unsigned short *>( t.c_str() ) ) );
    return res;
#else
    return QString::fromStdWString( t );
#endif
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
    return t.toStdWString();
#endif
}

