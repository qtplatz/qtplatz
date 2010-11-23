// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DATASUBSCRIBER_H
#define DATASUBSCRIBER_H

#include "adcontrols_global.h"

namespace adcontrols {

    class dataPublisher;
    class AcquiredDataset;
    class LCMSDataset;
    class ProcessedDataset;

    class ADCONTROLSSHARED_EXPORT dataSubscriber {  // visitable
    public:
        virtual ~dataSubscriber();
        dataSubscriber();

        virtual void subscribe( LCMSDataset& ) { }
        virtual void subscribe( ProcessedDataset& ) { }

    };

}

#endif // DATASUBSCRIBER_H
