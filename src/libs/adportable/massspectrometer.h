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
            virtual double getMass( double secs, int type ) const = 0;
            virtual double getTime( double mass, int type ) const = 0;
            virtual double getMass( double secs, double fLength ) const = 0;
            virtual double getTime( double mass, double fLength ) const = 0;
        };

        virtual const ScanLaw& getScanLaw() const = 0;
        static const MassSpectrometer& get( const std::wstring& modelname );
    };

    /////////////////////
    class MassSpectrometerBroker {
    protected:
        MassSpectrometerBroker();
    public:
        ~MassSpectrometerBroker();

        typedef MassSpectrometer *(*factory_type)(void);

        static MassSpectrometerBroker * instance();

        virtual bool install_factory( factory_type, const std::wstring& name ) = 0;
        virtual factory_type find( const std::wstring& name ) = 0;
    };


}
