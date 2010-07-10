//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "qstring.h"
#include <string>

using namespace qtwrapper;

#if defined _NATIVE_WCHAR_T_DEFINED

qstring::qstring( const std::wstring& t )
{
   
}

#else

qstring::qstring( const std::wstring& t ) : q_( QString::fromStdWString(t) )
{
}

#endif
