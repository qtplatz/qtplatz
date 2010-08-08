//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "qstring.h"
#include <adportable/utf.h>

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

