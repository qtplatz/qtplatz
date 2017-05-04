/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "msmoltable.hpp"
#include "mspeaks.hpp"
#include "mspeak.hpp"
#include "mspeakinfoitem.hpp"
#include "scanlaw.hpp"
#include <boost/uuid/uuid_generators.hpp>

using namespace adcontrols;

MSMolTable::~MSMolTable()
{
}

MSMolTable::MSMolTable() : acceleratorVoltage_( 0 )
                         , timeOffset_( 0 )
                         , hasCalibration_( false )
                         , mode_( 0 )
                         , fLength_( 0 )
                         , toleranceMethod_( ToleranceInDa )
                         , tolerances_( { 0.100, 0.1e-6, 5.0 } ) // 0.1Da, 0.1us, x5.0
{
    // tolerances_[ ToleranceInDa ] = 0.100; // 100mDa
    // tolerances_[ ToleranceInTime ] = 0.1e-6; // 0.1us
    // tolerances_[ ToleranceInPeakWidth ] = 5.0; 
}

MSMolTable::MSMolTable( const MSMolTable& t )
    : ident_( t.ident_ )
    , acceleratorVoltage_( t.acceleratorVoltage_ )
    , timeOffset_( t.timeOffset_ )
    , hasCalibration_( t.hasCalibration_ )
    , mode_( t.mode_ )
    , fLength_( t.fLength_ )
    , toleranceMethod_( t.toleranceMethod_ )
    , tolerances_( t.tolerances_ )
    , expected_( t.expected_ )
    , assigned_( t.assigned_ )
{
}

const idAudit&
MSMolTable::ident() const
{
    return ident_;
}

const boost::uuids::uuid&
MSMolTable::clsid()
{
    static boost::uuids::uuid __clsid = boost::uuids::string_generator()( "{5bdc69c6-a2fd-4df8-95cd-db5feaebd151}" );
    return __clsid;
}


double
MSMolTable::acceleratorVoltage() const
{
    return acceleratorVoltage_;
}

double
MSMolTable::timeOffset() const
{
    return timeOffset_;
}

bool
MSMolTable::hasCalibration() const
{
    return hasCalibration_;
}

int32_t
MSMolTable::mode() const
{
    return mode_;
}

double
MSMolTable::fLength() const
{
    return fLength_;
}

MSMolTable::eTolerance
MSMolTable::toleranceMethod() const
{
    return toleranceMethod_;
}

double
MSMolTable::tolerance( eTolerance m ) const
{
    if ( m >= nToleranceMethod )
        tolerances_[ this->toleranceMethod_ ];
    return tolerances_[ m ];
}

double
MSMolTable::tolerance() const
{
    return tolerances_[ this->toleranceMethod_ ];
}

void
MSMolTable::acceleratorVoltage( double v )
{
    acceleratorVoltage_ = v;
}

void
MSMolTable::timeOffset( double v )
{
    timeOffset_ = v;
}

void
MSMolTable::hasCalibration( bool v )
{
    hasCalibration_ = v;
}

void
MSMolTable::mode( int32_t v )
{
    mode_ = v;
}

void
MSMolTable::fLength( double v )
{
    fLength_ = v;
}

void
MSMolTable::toleranceMethod( eTolerance v )
{
    toleranceMethod_ = v;
}

void
MSMolTable::tolerance( eTolerance t, double v )
{
    tolerances_[ t ] = v;
}

const MSPeaks& 
MSMolTable::expected() const
{
    return expected_;
}

const MSPeaks&
MSMolTable::assigned() const
{
    return assigned_;
}

MSPeaks&
MSMolTable::expected()
{
    return expected_;
}

MSPeaks&
MSMolTable::assigned()
{
    return assigned_;
}

bool
MSMolTable::assignFormula( adcontrols::MSPeakInfoItem& pk, const adcontrols::ScanLaw& scanLaw, int mode ) const
{
    std::map< std::string, double > candidates;
    
    const auto& mols = this->expected();

    if ( ( toleranceMethod_ == ToleranceInPeakWidth ) || ( toleranceMethod_ == ToleranceInTime ) ) {

        double tolerance = this->tolerance();

        if ( toleranceMethod_ == ToleranceInPeakWidth )
            tolerance = ( pk.hh_right_time() - pk.hh_left_time() ) * this->tolerance() / 2.0;
    
        std::for_each( mols.begin(), mols.end(), [&]( auto& mol ){
                if ( mol.exact_mass() > 0.7 ) {
                    double tof = scanLaw.getTime( mol.exact_mass(), mode );
                    if ( std::abs( pk.time() - tof ) < tolerance )
                        candidates.emplace( mol.formula(), std::abs( tof - pk.time() ) );
                }
            });

    } else if ( toleranceMethod_ == ToleranceInDa ) {
        std::for_each( mols.begin(), mols.end(), [&]( auto& mol ){
                if ( std::abs( mol.exact_mass() - pk.mass() ) < this->tolerance() )
                    candidates.emplace( mol.formula(), std::abs( mol.exact_mass() - pk.mass() ) );
            });
    }

    if ( !candidates.empty() ) {
        auto it = std::min_element(candidates.begin(), candidates.end(), []( const auto& a, const auto& b ){ return a.second < b.second; } );
        pk.formula( it->first );
        return true;
    }

    return false;
}
