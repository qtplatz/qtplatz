// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"

namespace SignalObserver {
    struct DataReadBuffer;
}

namespace adcontrols {

    class MassSpectrum;
    class MassSpectrometer;

    class ADCONTROLSSHARED_EXPORT DataInterpreter {
    public:
        DataInterpreter(void);
        ~DataInterpreter(void);
        virtual bool translate( MassSpectrum&
                               , const SignalObserver::DataReadBuffer&
                               , const adcontrols::MassSpectrometer& ) const = 0;
    };

}
