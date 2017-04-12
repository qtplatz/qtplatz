/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "scanlaw.hpp"
#include "constants.hpp"
#include <adportable/float.hpp>
#include <cmath>
#include <cassert>

namespace multumcontrols {
    
    enum dim { LENGTH_L1
               , LENGTH_L2
               , LENGTH_L3
               , LENGTH_LG
               , LENGTH_L4
               , LENGTH_LT
               , LENGTH_EXIT };

    namespace infitof {   
        ScanLaw::~ScanLaw()
        {
        }
        
        ScanLaw::ScanLaw() : multumcontrols::ScanLaw( 3800.0, 0.0
                                                      , infitof::FLIGHT_LENGTH_L1
                                                      , infitof::FLIGHT_LENGTH_L2
                                                      , infitof::FLIGHT_LENGTH_L3
                                                      , infitof::FLIGHT_LENGTH_LG
                                                      , infitof::FLIGHT_LENGTH_L4
                                                      , infitof::FLIGHT_LENGTH_LT
                                                      , infitof::FLIGHT_LENGTH_EXIT )
        {
        }

        ScanLaw::ScanLaw( double kAcceleratorVoltage, double tDelay )
            : multumcontrols::ScanLaw( kAcceleratorVoltage, tDelay
                                       , infitof::FLIGHT_LENGTH_L1
                                       , infitof::FLIGHT_LENGTH_L2
                                       , infitof::FLIGHT_LENGTH_L3
                                       , infitof::FLIGHT_LENGTH_LG
                                       , infitof::FLIGHT_LENGTH_L4
                                       , infitof::FLIGHT_LENGTH_LT
                                       , infitof::FLIGHT_LENGTH_EXIT )
        {
        }

    }
}


using namespace multumcontrols;

ScanLaw::~ScanLaw()
{
}

ScanLaw::ScanLaw( double acceleratorVoltage
                  , double tDelay
                  , double _1, double _2, double _3
                  , double _4, double _5, double _6, double _7 )
    : adportable::TimeSquaredScanLaw( acceleratorVoltage, tDelay )
    , gateOffset_( 0 )
    , dimension_( { _1, _2, _3, _4, _5, _6, _7 } )
{
}

ScanLaw::ScanLaw( const ScanLaw& t )
    : adportable::TimeSquaredScanLaw( t )
    , gateOffset_( t.gateOffset_ )
    , dimension_( t.dimension_ )
{
}

ScanLaw&
ScanLaw::operator = ( const ScanLaw& t )
{
    static_cast< adportable::TimeSquaredScanLaw& >(*this) = static_cast< const adportable::TimeSquaredScanLaw >( t );
    
    gateOffset_ = t.gateOffset_;
    dimension_ = t.dimension_;
    return *this;
}

// TimeSquaredScanLaw
double
ScanLaw::tDelay() const
{
    return adportable::TimeSquaredScanLaw::tDelay();
}

double
ScanLaw::kAcceleratorVoltage() const
{
    return adportable::TimeSquaredScanLaw::kAcceleratorVoltage();
}

double
ScanLaw::acceleratorVoltage( double mass, double time, int mode, double tDelay )
{
    return adportable::TimeSquaredScanLaw::acceleratorVoltage( mass, time, mode, tDelay );
}

double
ScanLaw::getMass( double t, int mode ) const
{
    return adportable::TimeSquaredScanLaw::getMass( t, fLength( mode ) );
}

double
ScanLaw::getTime( double m, int mode ) const
{
    return adportable::TimeSquaredScanLaw::getTime( m, fLength( mode ) );    
}

double
ScanLaw::getMass( double t, double fLength ) const
{
    return adportable::TimeSquaredScanLaw::getMass( t, fLength );        
}

double
ScanLaw::getTime( double m, double fLength ) const
{
    return adportable::TimeSquaredScanLaw::getTime( m, fLength );
}

double
ScanLaw::fLength( int mode ) const
{
    return dimension_[ LENGTH_L1 ] + dimension_[ LENGTH_L2 ] + (dimension_[ LENGTH_LT ] * mode ) + dimension_[ LENGTH_EXIT ];    
}

double
ScanLaw::orbital_period( double mass ) const
{
    double k = ( adportable::kTimeSquaredCoeffs * kAcceleratorVoltage() ) / ( dimension_[ LENGTH_LT ] * dimension_[ LENGTH_LT ] );
	return std::sqrt(mass / k); // don't add tDealy for orbital period
}

double
ScanLaw::go_around_threshold_length() const
{
    return dimension_[ LENGTH_L1 ] + dimension_[ LENGTH_L2 ];
}

double
ScanLaw::go_around_threshold_time( double mass ) const
{
    return getTime( mass, dimension_[ LENGTH_L1 ] + dimension_[ LENGTH_L2 ] );
}

double
ScanLaw::exit_through_threshold_length( int nTurns ) const
{
    return dimension_[ LENGTH_L1 ] + dimension_[ LENGTH_L2 ] + dimension_[ LENGTH_LT ] * nTurns;
}

double
ScanLaw::exit_through_threshold_time( double mass, int nTurns ) const
{
    return getTime( mass, dimension_[ LENGTH_L1 ] + dimension_[ LENGTH_L2 ] + dimension_[ LENGTH_LT ] * nTurns );
}

double
ScanLaw::gate_through_threshold_length() const
{
    return dimension_[ LENGTH_L1 ] + ( dimension_[ LENGTH_LG ] + gateOffset_ );
}

double
ScanLaw::gate_through_threshold_time( double mass ) const
{
    return getTime( mass, dimension_[ LENGTH_L1 ] + ( dimension_[ LENGTH_LG ] + gateOffset_ ) );
}

//inline
double
ScanLaw::entry_through_threshold_length() const
{
    return dimension_[ LENGTH_L1 ];
}

double
ScanLaw::entry_through_threshold_time( double mass ) const
{
    return getTime( mass, dimension_[ LENGTH_L1 ] );
}

double
ScanLaw::number_of_turns( double exit_delay, double exact_mass ) const
{
    const double lap_threshold = go_around_threshold_time( exact_mass );
    int n = 0;
    if ( exit_delay >= lap_threshold )
        n = int( (exit_delay - lap_threshold) / orbital_period( exact_mass ) ) + 1;
    return n;
}

void
ScanLaw::setGateOffsetLength( double offset ) 
{
    gateOffset_ = offset;
}

double
ScanLaw::gateOffsetLength() const
{
    return gateOffset_;
}
