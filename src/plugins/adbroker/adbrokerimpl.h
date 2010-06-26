// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////
#ifndef ADBROKERIMPL_H
#define ADBROKERIMPL_H

#include "ADBroker.h"

  namespace internal {

    class ADBrokerImpl : public ADBroker {
      Q_OBJECT
    public:
      explicit ADBrokerImpl(QObject *parent = 0);
      
    signals:
      
    public slots:
      
    };

  }

#endif // ADBROKERIMPL_H
