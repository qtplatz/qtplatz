//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "ace_string.h"
#include <sstream>
#include <ace/INET_Addr.h>

using namespace acewrapper;

template<>
basic_string<char>::basic_string( const ACE_INET_Addr& addr )
{
   ACE_TCHAR tbuf[1024];
   memset(tbuf, 0, sizeof(tbuf));
   addr.addr_to_string( tbuf, sizeof(tbuf)/sizeof(tbuf[0]) );
   std::basic_ostringstream<char> o;
   o << tbuf;
   t_ = o.str();
}

template<>
basic_string<wchar_t>::basic_string( const ACE_INET_Addr& addr )
{
   ACE_TCHAR tbuf[1024];
   memset(tbuf, 0, sizeof(tbuf));
   addr.addr_to_string( tbuf, sizeof(tbuf)/sizeof(tbuf[0]) );
   std::basic_ostringstream<wchar_t> o;
   o << tbuf;
   t_ = o.str();
}
