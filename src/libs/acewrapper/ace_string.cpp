//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "ace_string.h"
#include <sstream>
#include <ace/INET_Addr.h>

using namespace acewrapper;

string::string( const ACE_INET_Addr& addr )
{
   ACE_TCHAR tbuf[1024];
   addr.addr_to_string( tbuf, sizeof(tbuf)/sizeof(tbuf[0]) );
   std::wostringstream o;
   o << tbuf;
   t_ = o.str();
}
