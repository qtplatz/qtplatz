//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "acceptor.h"
using namespace adcontrols;

Acceptor::~Acceptor(void)
{
}

bool
Acceptor::accept( Visitor& )
{
    return false;
}