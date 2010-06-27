//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "brokermanager.h"

BrokerManager * BrokerManager::instance_ = 0;

BrokerManager::~BrokerManager()
{
}

BrokerManager::BrokerManager()
{
}

// static
BrokerManager *
BrokerManager::instance()
{
  if ( instance_ == 0 )
	instance_ = new BrokerManager();
  return instance_;
}
