// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT PeakAsymmetry {
    public:
        PeakAsymmetry();

        double asymmetry() const;
        double startTime() const;
        double endTime() const;

    private:
        double peakAsymmetry_;
        double peakAsymmetryStartTime_;
        double peakAsymmetryEndTime_;
    };

}

