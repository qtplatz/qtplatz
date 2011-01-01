//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "MassSpectrometer.h"
#include "datainterpreter.h"
#include "MassSpectrometerBroker.h"
#include <string>
#include <cmath>

using namespace adcontrols;

namespace adcontrols {
    namespace internal {
		class ScanLaw : public MassSpectrometer::ScanLaw {
        public:
            ScanLaw( double timeCoefficient, double timeDelay, double acclVolt )
                : timeCoefficient_(timeCoefficient), timeDelay_(timeDelay), acclVoltage_(acclVolt) {
            }
            inline double getMass( double secs, int nTurn ) const { 
                return getMass( secs, 0.43764 + nTurn * 0.66273 );
            }
            inline double getTime( double mass, int nTurn ) const {
                return getTime( mass, 0.43764 + nTurn * 0.66273 );
            }
            double getMass( double secs, double fLength ) const {
                double t = secs / fLength - timeDelay_;
                double m = ( ( timeCoefficient_ * timeCoefficient_ ) * ( t * t ) ) * acclVoltage_;
                return m;
            }
            double getTime( double mass, double fLength ) const {
                double v = std::sqrt( acclVoltage_ / mass ) * timeCoefficient_; // (m/s)
                return fLength * ( 1.0 / v ) + timeDelay_;
            }
        private:
            double timeCoefficient_;
            double timeDelay_;
            double acclVoltage_;
        };

        class DataInterpreter : public adcontrols::DataInterpreter {
        public:
            DataInterpreter(void) {  }
            ~DataInterpreter(void) {  }
            virtual bool translate( adcontrols::MassSpectrum&, const SignalObserver::DataReadBuffer&
                , const adcontrols::MassSpectrometer&, size_t /* idData */ ) const {
                    return true;
            }
        };

        class MassSpectrometerImpl : public MassSpectrometer {
        public:
            MassSpectrometerImpl::~MassSpectrometerImpl() {
                delete impl_;
                impl_ = 0;
                delete scanLaw_;
                delete interpreter_;
            }
            MassSpectrometerImpl() {
                scanLaw_ = new internal::ScanLaw( 0.01389, 0.0, 5000 );
                interpreter_ = new internal::DataInterpreter();
            }
            virtual void accept( adcontrols::Visitor& ) {
            }
            virtual adcontrols::MassSpectrometer::factory_type factory() {
                return &instance;
            }
            virtual const wchar_t * name() const {
                return L"default";
            }
            virtual const MassSpectrometer::ScanLaw& getScanLaw() const {
                return * scanLaw_;
            }
            virtual const adcontrols::DataInterpreter& getDataInterpreter() const {
                return * interpreter_;
            }
        private:
            static adcontrols::MassSpectrometer * instance() {
                if ( ! impl_ )
                    impl_ = new MassSpectrometerImpl;
                return impl_;
            }
            internal::ScanLaw * scanLaw_;
            internal::DataInterpreter * interpreter_;
            static MassSpectrometerImpl * impl_;
        };
    }
}

internal::MassSpectrometerImpl * internal::MassSpectrometerImpl::impl_ = 0;


///////////////////////////////////////////////

const MassSpectrometer&
MassSpectrometer::get( const std::wstring& modelname )
{
	MassSpectrometerBroker::factory_type factory = MassSpectrometerBroker::find( modelname );
	if ( factory )
		return *factory();
    static internal::MassSpectrometerImpl dummy;
    return dummy;
    // throw std::exception("mass spectrometer not registered. Check servant.config.xml or configloader");
}

//////////////////////////////////////////////////////////////