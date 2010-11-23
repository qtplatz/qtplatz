// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"
#include "acquireddataset.h"

namespace adcontrols {

    class Chromatogram;
    class MassSpectrum;

    class LCMSDataset : public AcquiredDataset {
    public:
        // LCMSDataset();
        virtual size_t getFunctionCount() const = 0;
        virtual size_t getSpectrumCount( int fcn ) const = 0;
        virtual size_t getChromatogramCount() const = 0;
        virtual bool getTIC( int fcn, adcontrols::Chromatogram& ) const = 0;
        virtual bool getSpectrum( int fcn, int idx, adcontrols::MassSpectrum& ) const = 0;
    };

}


