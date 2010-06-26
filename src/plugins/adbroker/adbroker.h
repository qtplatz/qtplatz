// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#pragma once

#include <QObject>
#include "adbroker_global.h"
#include "modelfacade.h"
#include "analysismodel.h"
#include "acquiremodel.h"

class ModelFacade;

class ADBROKER_EXPORT ADBroker : public QObject {
  Q_OBJECT
public:
    explicit ADBroker(QObject *parent = 0);
    static ADBroker * instance();

    template<class T> T * getModel() {
        if ( modelfacade_ )
            return modelfacade_->getModel<T>();
        return 0;
    }
  
signals:
  
public slots:
  
 private:
     ModelFacade * modelfacade_;
     static ADBroker * instance_;
  
};

