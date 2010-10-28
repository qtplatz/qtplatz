// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"

namespace adcontrols {

    class Acceptor;
    class MassSpectrometer;

    class ADCONTROLSSHARED_EXPORT Visitor {
    public:
        Visitor(void);
        virtual ~Visitor(void);
        virtual void visit( Acceptor& ) {}
        virtual void visit( MassSpectrometer& ) {}
    };

}
