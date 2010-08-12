//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "tofcontroller.h"


TofController::~TofController()
{
}

TofController::TofController()
{
}

bool
TofController::initialize( CORBA::ORB * orb )
{
    return false;
}

bool
TofController::activate()
{
    return false;
}

bool
TofController::deactivate()
{
    return false;
}

int
TofController::run()
{
    return 0;
}

void
TofController::abort_server()
{
}

void
TofController::dispose()
{
    delete this;
}

adplugin::orbLoader * instance()
{
    return new TofController();
}
