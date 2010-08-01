//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "session_i.h"

using namespace broker;

session_i::session_i(void)
{
}

session_i::~session_i(void)
{
}

bool
session_i::connect( const char * user, const char * pass, const char * token )
{
    ACE_UNUSED_ARG( user );
    ACE_UNUSED_ARG( pass );
    ACE_UNUSED_ARG( token );
    return false;
}
