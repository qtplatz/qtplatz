//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "ibroker.h"
#include "oproxy.h"
/*
#include <acewrapper/mutex.hpp>
#include "ibrokermanager.h"
#include "message.h"
#include <acewrapper/timeval.h>
#include <acewrapper/messageblock.h>
#include <iostream>
#include <sstream>
#include <adinterface/eventlog_helper.h>
#include "marshal.hpp"
#include "constants.h"
#include <adportable/configuration.h>
#include <adportable/configloader.h>
*/

using namespace adcontroller;

oProxy::~oProxy()
{
}

oProxy::oProxy( iBroker& t ) : broker_( t )
{
}

void
oProxy::OnUpdateData ( ::CORBA::Long pos )
{
}

void
oProxy::OnMethodChanged ( ::CORBA::Long pos )
{
}

void
oProxy::OnEvent ( ::CORBA::ULong event,	::CORBA::Long pos )
{
}

