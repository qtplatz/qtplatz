// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "session.h"

#include <adinterface/controlserverC.h>
#include <acewrapper/mutex.hpp>

using namespace acquire::internal;

Session::Session() 
{
}

bool
Session::initialize()
{
	return false;
}
