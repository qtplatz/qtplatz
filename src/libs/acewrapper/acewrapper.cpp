//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "acewrapper.h"
#include <ace/Init_ACE.h>
#include <ace/Singleton.h>

using namespace acewrapper;

instance_manager::instance_manager()
{
    ACE::init();
}

instance_manager::~instance_manager()
{
    ACE::fini();
}

void
instance_manager::initialize_i()
{
    // do nothing
}

void
instance_manager::finalize_i()
{
    // do nothing
}

void
instance_manager::initialize()
{
    instance_manager_t::instance()->initialize_i();
}

void
instance_manager::finalize()
{
    instance_manager_t::instance()->finalize_i();
}
