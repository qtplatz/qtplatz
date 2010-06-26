//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "adbroker.h"
#include "adbrokerimpl.h"

ADBroker * ADBroker::instance_ = 0;

ADBroker::ADBroker(QObject *parent) : QObject(parent)
{
}

ADBroker *
ADBroker::instance()
{
  if ( instance_ == 0 )
    instance_ = new internal::ADBrokerImpl;
  return instance_;
}
