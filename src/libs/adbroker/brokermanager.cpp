//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "brokermanager.h"
#include "task.h"

using namespace adbroker;

// BrokerManager * BrokerManager::instance_ = 0;
bool BrokerManager::initialized_ = false;

BrokerManager::~BrokerManager()
{
    delete pTask_;
}

BrokerManager::BrokerManager() : pTask_(0)
{
    initialized_ = true;
    pTask_ = new Task(5);
    initialize();
}

bool
BrokerManager::initialize()
{
    pTask_->open();
    return true;
}

// static
void
BrokerManager::terminate()
{
    if ( initialized_ )
        singleton::BrokerManager::instance()->pTask_->close();
}
