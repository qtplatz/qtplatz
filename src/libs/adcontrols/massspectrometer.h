// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"
#include <string>

namespace adcontrols {

    class Visitor;
    class DataInterpreter;
    
    class ADCONTROLSSHARED_EXPORT MassSpectrometer {
    public:
        MassSpectrometer(void) {}
        virtual ~MassSpectrometer(void) {}
        
        class ScanLaw {
        public:
            virtual double getMass( double secs, int type ) const = 0;
            virtual double getTime( double mass, int type ) const = 0;
            virtual double getMass( double secs, double fLength ) const = 0;
            virtual double getTime( double mass, double fLength ) const = 0;
        };
        typedef MassSpectrometer * (*factory_type)(void);
        
        virtual void accept( Visitor& ) = 0;
        virtual factory_type factory() = 0;
        virtual const wchar_t * name() const = 0;
        virtual const ScanLaw& getScanLaw() const = 0;
        virtual const DataInterpreter& getDataInterpreter() const = 0;
        
        static const MassSpectrometer& get( const std::wstring& modelname );
    };
    
}

