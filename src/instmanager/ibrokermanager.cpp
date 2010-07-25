//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "IBrokerManager.h"
#include "ibroker.h"
#include <boost/noncopyable.hpp>

///////////////////////////////////////////////////////////////////

IBrokerManager::~IBrokerManager()
{
    delete pBroker_;
}

IBrokerManager::IBrokerManager() : pBroker_(0)
{
    pBroker_ = new iBroker();
}

