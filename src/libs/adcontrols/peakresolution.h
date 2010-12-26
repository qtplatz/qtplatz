// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT PeakResolution {
    public:
        PeakResolution();

        double Rs() const;
        double RsBaselineStartTime() const;
        double RsBaselineStartHeight() const;
        double RsBaselineEndTime() const;
        double RsBaselineEndHeight() const;
        double RsPeakTopTime() const;
        double RsPeakTopHeight() const;

    private:
        double rs_;
        double rsBaselineStartTime_;
        double rsBaselineStartHeight_;
        double rsBaselineEndTime_;
        double rsBaselineEndHeight_;
        double rsPeakTopTime_;
        double rsPeakTopHeight_;
    };

}

