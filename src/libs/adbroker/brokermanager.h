// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef ADBROKERMANAGER_H
#define ADBROKERMANAGER_H

#include "adbroker_global.h"

class BrokerSession;
class BrokerAccessToken;
class BrokerConfig;

namespace adil {
  class ElementIO;
}

class ADBROKERSHARED_EXPORT BrokerManager {
  virtual ~BrokerManager();
  BrokerManager();
 public:
  static BrokerManager * instance();

  BrokerSession * getBrokerSession();
  adil::ElementIO& getElementIO();
  
 private:
  static BrokerManager * instance_;
};

#endif // ADBROKERMANAGER_H
