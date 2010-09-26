//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "massspectrometer.h"
#include <stdlib.h>
#include <cmath>
#include <map>

using namespace adportable;

namespace adportable {
    namespace internal {
        class TimeSquaredScanLaw;
        class TimeLinearScanLaw;
        class TOFTimeSquaredScanLawSpectrometer;
    }
}

namespace adportable {
    namespace internal {

        class InfiTOF : public MassSpectrometer {
            ~InfiTOF();
            InfiTOF();
        public:
            static MassSpectrometer * instance();
            static void dispose();
            const MassSpectrometer::ScanLaw& getScanLaw() const;
        private:
            static InfiTOF * instance_;
            MassSpectrometer::ScanLaw * pScanLaw_;
        };

        class TimeSquaredScanLaw : public MassSpectrometer::ScanLaw {
        public:
            TimeSquaredScanLaw( double timeCoefficient, double timeDelay, double acclVolt );
            double getMass( double secs, int type ) const;
            double getTime( double mass, int type ) const;
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
        private:
            double timeCoefficient_;
            double timeDelay_;
            double acclVoltage_;
        };

    }
}

////////////////////////////////////////
MassSpectrometer::MassSpectrometer(void)
{
}

MassSpectrometer::~MassSpectrometer(void)
{
}

const MassSpectrometer&
MassSpectrometer::get( const std::wstring& modelname )
{
    MassSpectrometerBroker::factory_type factory = MassSpectrometerBroker::instance()->find( modelname );
    // next two line demonstrate how to install MassSpectrometer class newly developped outside qtPlatz public source
    if ( ! factory && modelname == L"InfiTOF" ) {
        MassSpectrometerBroker::instance()->install_factory( internal::InfiTOF::instance, L"InfiTOF" );
        factory = MassSpectrometerBroker::instance()->find( modelname );
    }

    if ( factory )
        return *factory();

    throw std::exception("unknown mass spectrometer");
}

////////////////////////////////////////////////

using namespace adportable::internal;

////////////////////////////////////////////////
InfiTOF * InfiTOF::instance_ = 0;

MassSpectrometer *
InfiTOF::instance()
{
    if ( instance_ == 0 ) {
        instance_ = new InfiTOF();
        atexit( InfiTOF::dispose );
    }
    return instance_;
}

void
InfiTOF::dispose()
{
   if ( instance_ )
       delete instance_;
   instance_ = 0;
}

InfiTOF::~InfiTOF()
{
    delete pScanLaw_;
}

InfiTOF::InfiTOF() : pScanLaw_(0)
{
    pScanLaw_ = new MultiTurnScanLaw( 0.01389, /* delay */ 0.0, /* FT(V) */ 5000 );
}

const MassSpectrometer::ScanLaw&
InfiTOF::getScanLaw() const
{
    return *pScanLaw_;
}


///////////////////////////////////////////////

TimeSquaredScanLaw::TimeSquaredScanLaw( double timeCoefficient, double timeDelay, double acclVolt )
: timeCoefficient_( timeCoefficient )
, timeDelay_( timeDelay )
, acclVoltage_( acclVolt )
, flen_(1.0)
{
}

double
TimeSquaredScanLaw::getMass( double tof, int ) const
{
	double t = tof / flen_ - timeDelay_;
    double m = ( ( timeCoefficient_ * timeCoefficient_ ) * ( t * t ) ) * acclVoltage_;
    return m;
}

double
TimeSquaredScanLaw::getTime( double mass, int ) const
{
    double v = std::sqrt( acclVoltage_ / mass ) * timeCoefficient_; // (m/s)
	return flen_ * ( 1.0 / v ) + timeDelay_;  // time(us) for 1m flight pass
}

/////////////////////////////////////////////

MultiTurnScanLaw::MultiTurnScanLaw( double timeCoefficient, double timeDelay, double acclVolt )
: timeCoefficient_( timeCoefficient )
, timeDelay_( timeDelay )
, acclVoltage_( acclVolt )
{
}

double
MultiTurnScanLaw::getMass( double tof, int nTurn ) const
{
    double L = 0.43764 + nTurn * 0.66273;
    double t = tof / L - timeDelay_;
    double m = ( ( timeCoefficient_ * timeCoefficient_ ) * ( t * t ) ) * acclVoltage_;
    return m;
}

double
MultiTurnScanLaw::getTime( double mass, int nTurn ) const
{
    double L = 0.43764 + nTurn * 0.66273;
    double v = std::sqrt( acclVoltage_ / mass ) * timeCoefficient_; // (m/s)
    return L * ( 1.0 / v ) + timeDelay_;
}

/////////////////////////////////////////////

MassSpectrometerBroker::MassSpectrometerBroker()
{
}

MassSpectrometerBroker::~MassSpectrometerBroker()
{
}

namespace adportable {
    namespace internal {
        class MassSpectrometerBrokerImpl : public MassSpectrometerBroker {
        public:
            ~MassSpectrometerBrokerImpl() {}

            static MassSpectrometerBroker * instance() {
                if ( instance_ == 0 ) {
                    instance_ = new MassSpectrometerBrokerImpl();
                    atexit( MassSpectrometerBrokerImpl::dispose );
                }
                return instance_;
            }

            static void dispose() { 
                delete instance_;
            }

            bool install_factory( factory_type factory, const std::wstring& name ) {
                factories_[name] = factory;
                return true;
            }

            factory_type find( const std::wstring& name ) {
                std::map< std::wstring, factory_type >::iterator it = factories_.find( name );
                if ( it != factories_.end() )
                    return it->second;
                return 0;
            }

        private:
            std::map< std::wstring, factory_type > factories_;
            static MassSpectrometerBroker * instance_;
        };
    }
}

MassSpectrometerBroker * MassSpectrometerBrokerImpl::instance_ = 0;

MassSpectrometerBroker *
MassSpectrometerBroker::instance()
{
    return MassSpectrometerBrokerImpl::instance();
}
