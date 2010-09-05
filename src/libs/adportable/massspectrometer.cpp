//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "massspectrometer.h"
#include <stdlib.h>
#include <cmath>

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
            static InfiTOF * instance();
            static void dispose();
            const MassSpectrometer::ScanLaw& getScanLaw() const;
        private:
            static InfiTOF * instance_;
            MassSpectrometer::ScanLaw * pScanLaw_;
        };

        class TimeSquaredScanLaw : public MassSpectrometer::ScanLaw {
        public:
            TimeSquaredScanLaw( double timeCoefficient, double timeDelay, double acclVolt );
            double getMass( double secs ) const;
            double getTime( double mass ) const;
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
    if ( modelname == L"InfiTOF" ) {
        return * internal::InfiTOF::instance();
    }
    throw std::exception("unknown mass spectrometer");
}

////////////////////////////////////////////////

using namespace adportable::internal;

////////////////////////////////////////////////
InfiTOF * InfiTOF::instance_ = 0;

InfiTOF *
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
    pScanLaw_ = new TimeSquaredScanLaw( 0.01389, 0.0, 5000 );
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
{
}

double
TimeSquaredScanLaw::getMass( double secs ) const
{
    double t = secs - timeDelay_;
    double m = ( ( timeCoefficient_ * timeCoefficient_ ) * ( t * t ) ) * acclVoltage_;
    return m;
}

double
TimeSquaredScanLaw::getTime( double mass ) const
{
    double v = std::sqrt( acclVoltage_ / mass ) * timeCoefficient_; // (m/s)
    return ( 1.0 / v ) + timeDelay_;  // time(us) for 1m flight pass
}
