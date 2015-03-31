/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "waveform_generator.hpp"
#include <adportable/timesquaredscanlaw.hpp>
#include <adportable/float.hpp>

#include <adcontrols/metric/prefix.hpp>
#include <adlog/logger.hpp>
using namespace adcontrols::metric;

#include <boost/math/distributions/normal.hpp>
#include <boost/random.hpp>
#include <chrono>
#include <sstream>

std::chrono::high_resolution_clock::time_point __uptime__ = std::chrono::high_resolution_clock::now();
std::chrono::high_resolution_clock::time_point __last__;
static uint32_t __serialNumber__;
//static const adportable::TimeSquaredScanLaw scanLaw;

namespace u5303a {

    struct noise {

        boost::mt19937 gen;

        boost::uniform_real<> dist;
        noise( double min, double max ) : dist( min, max ) {}
        double operator()() {
            boost::variate_generator< boost::mt19937&, boost::uniform_real<> > rand( gen, dist );
            return rand();
        };
    };

}


using namespace u5303a;

waveform_generator::waveform_generator( std::shared_ptr< adportable::TimeSquaredScanLaw >& law
                                        , double sampInterval
                                        , double startDelay
                                        , uint32_t nbrSamples
                                        , uint32_t nbrWaveforms ) : scanLaw_( law )
                                                                  , sampInterval_( sampInterval )
                                                                  , startDelay_( startDelay )
                                                                  , nbrSamples_( nbrSamples )
                                                                  , timeStamp_( 0 )
                                                                  , nbrWaveforms_( nbrWaveforms )
                                                                  , waveform_( nbrSamples_ )
                                                                  , serialNumber_( __serialNumber__++ )
{
}

void
waveform_generator::addIons( const std::vector< std::pair< double, double> >& ions )
{
    ions_.clear();

    for ( auto& ion: ions ) {
        ion_t t;
        t.mass = ion.first;
        t.height = ion.second;
        t.width = 5e-9; // 5ns
        ions_.push_back( t );
    }
}


void
waveform_generator::onTriggered()
{
	std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();    
    timeStamp_ = std::chrono::duration< double >( now - __uptime__ ).count(); // s

    noise __noise__( -15.0, 35.0 );

    using namespace adcontrols::metric;

    double seconds = timeStamp_;
    double hf = ( std::sin( seconds / 10.0 ) + 1.20 ) / 2.20;

    std::vector< double > times;

	static int counter;

    std::ostringstream o;
    auto d = std::chrono::duration< double >( now - __last__ ).count();
    o << "onTriggerd: " << counter++ << " t: " << seconds << " " << int( d * 1000 ); // ms
	__last__ = now;

    for ( const auto& ion: ions_ ) {
        double t = scanLaw_->getTime( ion.mass, int(0) );
        times.push_back( t );
        o << "\tm/z=" << std::setprecision(4) << std::fixed << ion.mass
          << " tof=" << scale_to_micro(t);
    }
    ADINFO() << o.str();

    size_t idx = 0;
    for ( int32_t& d: waveform_ ) {

        double t = startDelay_ + sampInterval_ * idx++;

        auto itIon = ions_.begin();
        auto itTime = times.begin();
        double y = 0;
        while ( itIon != ions_.end() && itTime != times.end() ) {
            boost::math::normal_distribution< double > nd( *itTime /* mean */, itIon->width /* sd */); // 10ns width
            y += boost::math::pdf( nd, t ) / boost::math::pdf( nd, *itTime ) * itIon->height * hf + __noise__();
            ++itIon;
            ++itTime;
        }
        d = int32_t(y) + 10000; // add background (simulate high background level)
    }
}

double
waveform_generator::mass_to_time( double mass, int nTurn )
{
    return scanLaw_->getTime( mass, nTurn );
}

const std::vector< int32_t >&
waveform_generator::waveform() const
{
    return waveform_;
}

double
waveform_generator::timestamp() const
{
    return timeStamp_;
}

uint32_t
waveform_generator::serialNumber() const
{
    return serialNumber_;
}

double
waveform_generator::startDelay() const
{
    return startDelay_;
}

uint32_t
waveform_generator::nbrWaveforms() const
{
    return nbrWaveforms_;
}

uint32_t
waveform_generator::nbrSamples() const
{
    return nbrSamples_;
}

double
waveform_generator::sampInterval() const
{
    return sampInterval_;
}
