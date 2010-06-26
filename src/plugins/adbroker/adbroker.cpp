//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "adbroker.h"
//#include "adbrokerimpl.h"
#include "modelfacade.h"

ADBroker * ADBroker::instance_ = 0;

ADBroker::ADBroker(QObject *parent) : QObject(parent)
                                    , modelfacade_(0) 
{
    modelfacade_ = new ModelFacade();
    if ( modelfacade_ ) {
        modelfacade_->setModel<AcquireModel>( AcquireModel() );
        modelfacade_->setModel<AnalysisModel>( AnalysisModel() );
    }
}

ADBroker *
ADBroker::instance()
{
    if ( instance_ == 0 )
        instance_ = new ADBroker();
    return instance_;
}
