//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "adbroker.h"

ADBroker * ADBroker::instance_ = 0;

ADBroker::ADBroker(QObject *parent) : QObject(parent)
{
}

ADBroker *
ADBroker::instance()
{
  return 0;
}
