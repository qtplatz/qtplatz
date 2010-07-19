//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "roleaverager.h"

RoleAverager::~RoleAverager()
{
}

RoleAverager::RoleAverager()
{
}

RoleAverager::RoleAverager( const RoleAverager& )
{
}

// trigger disarmed after current averaging
bool
RoleAverager::instruct_average_stop( handleIt& handler )
{
    handler( true ); // acknowlege
    return true;
}

// trigger armed immediately
bool
RoleAverager::instruct_average_start()
{
    return true;
}