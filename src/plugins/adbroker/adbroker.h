// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#pragma once

#include <QObject>
#include "adbroker_global.h"

class ADBROKER_EXPORT ADBroker : public QObject {
  Q_OBJECT
public:
  explicit ADBroker(QObject *parent = 0);
  static ADBroker * instance();
  
 signals:
  
  public slots:
  
 private:
  static ADBroker * instance_;
  
};

