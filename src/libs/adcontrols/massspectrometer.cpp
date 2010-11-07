//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "MassSpectrometer.h"
#include "MassSpectrometerBroker.h"
#include <string>

using namespace adcontrols;

namespace adportable {

    namespace internal {

        class TimeSquaredScanLaw : public MassSpectrometer::ScanLaw {
        public:
            TimeSquaredScanLaw( double timeCoefficient, double timeDelay, double acclVolt );
            double getMass( double secs, int type ) const;
            double getTime( double mass, int type ) const;
            double getMass( double secs, double L ) const;
            double getTime( double mass, double L ) const;
        private:
            double timeCoefficient_;
            double timeDelay_;
            double acclVoltage_;
            double flen_;
        };

        class MultiTurnScanLaw : public MassSpectrometer::ScanLaw {
        public:
            MultiTurnScanLaw( double timeCoefficient, double timeDelay, double acclVolt );
            double getMass( double secs, int nTurn ) const;
            double getTime( double mass, int nTurn ) const;
            double getMass( double secs, double fLength ) const;
            double getTime( double mass, double fLength ) const;
        private:
            double timeCoefficient_;
            double timeDelay_;
            double acclVoltage_;
        };

    }
}

///////////////////////////////////////////////

const MassSpectrometer&
MassSpectrometer::get( const std::wstring& modelname )
{
	MassSpectrometerBroker::factory_type factory = MassSpectrometerBroker::find( modelname );
	if ( factory )
		return *factory();
	throw std::exception("mass spectrometer not registered. Check servant.config.xml or configloader");
}

//////////////////////////////////////////////////////////////