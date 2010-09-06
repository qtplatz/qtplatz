// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <string>

namespace adportable {

    class MassSpectrometer {
    public:
        MassSpectrometer(void);
        ~MassSpectrometer(void);

        class ScanLaw {
        public:
            virtual double getMass( double secs, double flen = 1.0 ) const = 0;
            virtual double getTime( double mass, double flen = 1.0 ) const = 0;
        };

        virtual const ScanLaw& getScanLaw() const = 0;
        static const MassSpectrometer& get( const std::wstring& modelname );
    };

}
