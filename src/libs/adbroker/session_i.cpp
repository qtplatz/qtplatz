//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "session_i.h"
#include "brokermanager.h"

using namespace adbroker;

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

    adbroker::Task * pTask = adbroker::singleton::BrokerManager::instance()->get<adbroker::Task>();
    if ( pTask )
		return true;
    return false;
}
