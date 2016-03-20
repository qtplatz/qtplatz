// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "lockmass.hpp"
#include "annotation.hpp"
#include "annotations.hpp"
#include "chemicalformula.hpp"
#include "massspectrum.hpp"
#include "mspeakinfo.hpp"
#include "mspeakinfoitem.hpp"
#include <adportable/polfit.hpp>
#include <adportable/float.hpp>

using namespace adcontrols;
using namespace adcontrols::lockmass;

mslock::mslock()
{
}

mslock::mslock( const mslock& t ) : references_( t.references_ )
{
}

mslock::operator bool () const
{
    return !references_.empty();
}

mslock&
mslock::operator << ( const reference& t )
{
    if ( ! references_.empty() ) {

        auto it = std::find_if( references_.begin(), references_.end(), [t]( const reference& a ){
                return adportable::compare<double>::essentiallyEqual( t.exactMass(), a.exactMass() ); });

        if ( it != references_.end() )
            references_.erase( it );

    }
    
    references_.push_back( t );
    
	return *this;
}

reference::reference() : exactMass_(0)
                       , matchedMass_(0)
                       , time_(0)
{
}

reference::reference( const reference& t ) : formula_( t.formula_ )
                                           , exactMass_( t.exactMass_ )
                                           , matchedMass_( t.matchedMass_ )
                                           , time_( t.time_ )
{
}

reference::reference( const std::string& formula
                      , double exactMass
                      , double matchedMass
                      , double time ) : formula_( formula )
                                      , exactMass_( exactMass )
                                      , matchedMass_( matchedMass )
                                      , time_( time )
{
}

const std::string&
reference::formula() const
{
    return formula_;
}

double
reference::exactMass() const
{
    return exactMass_;
}

double
reference::matchedMass() const
{
    return matchedMass_;
}

double
reference::time() const
{
    return time_;
}

//////////////////////
void
mslock::clear()
{
    references_.clear();
}

size_t
mslock::size() const
{
    return references_.size();
}

bool
mslock::empty() const
{
    return references_.empty();
}

mslock::const_iterator 
mslock::begin() const
{
    return references_.begin();
}

mslock::const_iterator
mslock::end() const
{
    return references_.end();
}

const std::vector< double >&
mslock::coeffs() const
{
    return fitter_.coeffs();
}

const lockmass::fitter&
mslock::fitter() const
{
    return fitter_;
}

///////////////////

// static
bool
mslock::findReferences( mslock& lk,  const adcontrols::MassSpectrum& ms )
{
    static ChemicalFormula formulaParser;

    lk.clear();
    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( ms );
    for ( auto& fms: segs ) {
        for ( const auto& a: fms.get_annotations() ) {
            if ( a.dataFormat() == annotation::dataFormula && a.index() >= 0 ) {
                const std::string& formula = a.text();
                double exactMass = formulaParser.getMonoIsotopicMass( formula );
                double matchedMass = fms.getMass( a.index() );
                double time = fms.getTime( a.index() );
                lk << reference( formula, exactMass, matchedMass, time );
            }
        }
    }
    return true;
}

// static
bool
mslock::findReferences( mslock& lk,  const adcontrols::MassSpectrum& ms, int idx, int fcn )
{
    static ChemicalFormula formulaParser;

    if ( idx < 0 || fcn < 0 )
        return false;

    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( ms );
    if ( size_t(fcn) >= segs.size() )
        return false;

    auto& annots = segs[ fcn ].get_annotations();

    auto it = std::find_if( annots.begin(), annots.end(), [=]( const adcontrols::annotation& a ){ return a.index() == idx; });
    if ( it != annots.end() ) {
        const std::string& formula = it->text();
        auto list = adcontrols::ChemicalFormula::split( formula );
        double exactMass = formulaParser.getMonoIsotopicMass( list );
        double matchedMass = segs[ fcn ].getMass( it->index() );
        double time        = segs[ fcn ].getTime( it->index() );
        lk << reference( formula, exactMass, matchedMass, time );        

        return true;
    }
    return false;
}

bool
mslock::fit()
{
    fitter_.clear();
    
    if ( references_.size() == 1 ) {
        auto& ref = references_[0];
        double error = ref.matchedMass() - ref.exactMass();
        double relativeError = error / ref.matchedMass();

        fitter_ = std::vector< double >{  relativeError };
        return true;

    } else if ( references_.size() >= 2 ) {

        std::vector< double > x, y;

        for ( const auto& ref: references_ ) {
            double error = ref.matchedMass() - ref.exactMass();
            x.push_back( ref.matchedMass() );
            y.push_back( error );
        }
        return adportable::polfit::fit( x.data(), y.data(), x.size(), 2, fitter_.coeffs() );
    }

    return false;
}

bool
mslock::operator()( MassSpectrum& ms, bool applyToAll ) const
{
    if ( applyToAll ) {
        std::pair< double, double > range(1000000.0, 0.0);

        for ( auto& fms : adcontrols::segment_wrapper<>( ms ) ) {
            fitter_( fms );
            range.first = std::min( fms.getMass( 0 ), range.first );
            range.second = std::max( fms.getMass( fms.size() - 1 ), range.second );
        }

        range.first = range.first - ( range.second - range.first ) / 100;
        range.second = range.second + ( range.second - range.first ) / 100;

        ms.setAcquisitionMassRange( range.first, range.second );

        return true;

    } else
        return fitter_( ms );
}

bool
mslock::operator()( MSPeakInfo& info, bool applyToAll ) const
{
    if ( applyToAll ) {
        for ( auto& xinfo : adcontrols::segment_wrapper< MSPeakInfo >( info ) )
            fitter_( xinfo );
        return true;
    } else
        return fitter_( info );
}

fitter::fitter()
{
}

fitter::fitter( const fitter& t ) : coeffs_( t.coeffs_ )
{
}

fitter::fitter( std::vector< double >&& a )
{
    coeffs_ = std::move( a );
}

fitter&
fitter::operator = ( std::vector< double >&& a )
{
    coeffs_ = std::move( a );   
    return *this;
}

const std::vector< double >& 
fitter::coeffs() const
{
    return coeffs_;
}

std::vector< double >& 
fitter::coeffs()
{
    return coeffs_;
}

bool
fitter::operator()( MassSpectrum& ms ) const
{
    if ( coeffs_.empty() )
        return false;
    
    const double * masses = ms.getMassArray();

    if ( coeffs_.size() == 1 ) {
        // relative error correction
        for ( size_t i = 0; i < ms.size(); ++i ) {
            double mass = masses[ i ] - masses[ i ] * coeffs_[ 0 ];
            ms.setMass( i, mass );
        }

    } else {

        for ( size_t i = 0; i < ms.size(); ++i ) {
            double mass = masses[i] - adportable::polfit::estimate_y( coeffs_, masses[ i ] );
            ms.setMass( i, mass );
        }

    }

	return true;
}

bool
fitter::operator()( MSPeakInfo& pkInfo ) const
{
    if ( coeffs_.empty() )
        return false;

    if ( coeffs_.size() == 1 ) {
        // relative error correction
        for ( auto& item: pkInfo ) {
            double mass = item.mass() - item.mass() * coeffs_[ 0 ];
            item.assign_mass( mass );
        }

    } else {

        for ( auto& item: pkInfo ) { 
            double mass = item.mass() - adportable::polfit::estimate_y( coeffs_, item.mass() );
            item.assign_mass( mass );
        }

    }

	return true;
}

void
fitter::clear()
{
    coeffs_.clear();
}

