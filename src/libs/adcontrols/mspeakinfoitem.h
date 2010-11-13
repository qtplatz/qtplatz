// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT MSPeakInfoItem {
    public:
        ~MSPeakInfoItem(void);
        MSPeakInfoItem(void);
        MSPeakInfoItem( const MSPeakInfoItem& );
        MSPeakInfoItem( double mass, double area, double height, double hh );
        double mass();
        double area();
        double height();
        double widthHH();

    private:
        double mass_;
        double area_;
        double height_;
        double hh_;
    };

}
