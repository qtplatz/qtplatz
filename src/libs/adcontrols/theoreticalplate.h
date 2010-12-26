// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"


namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT TheoreticalPlate {
    public:
        TheoreticalPlate();

        double NTP() const;
        double NTPBaselineStartTime() const;
        double NTPBaselineStartHeight() const;
        double NTPBaselineEndTime() const;
        double NTPBaselineEndHeight() const;
        double NTPPeakTopTime() const;
        double NTPPeakTopHeight() const;

    private:
        double ntp_;
        double ntpBaselineStartTime_;
        double ntpBaselineStartHeight_;
        double ntpBaselineEndTime_;
        double ntpBaselineEndHeight_;
        double ntpPeakTopTime_;
        double ntpPeakTopHeight_;
    };

}


