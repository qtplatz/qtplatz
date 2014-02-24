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
#include "massspectrum.hpp"
#include "chemicalformula.hpp"
#include "annotations.hpp"
#include "annotation.hpp"
#include <adportable/polfit.hpp>

using namespace adcontrols;

lockmass::lockmass()
{
}

lockmass::lockmass( const lockmass& t ) : references_( t.references_ )
{
}

lockmass::operator bool () const
{
    return !references_.empty();
}

lockmass&
lockmass::operator << ( const lockmass::reference& t )
{
    references_.push_back( t );
	return *this;
}

lockmass::reference::reference() : exactMass_(0)
                                 , matchedMass_(0)
                                 , time_(0)
{
}

lockmass::reference::reference( const lockmass::reference& t ) : formula_( t.formula_ )
                                                               , exactMass_( t.exactMass_ )
                                                               , matchedMass_( t.matchedMass_ )
                                                               , time_( t.time_ )
{
}

lockmass::reference::reference( const std::string& formula
                                , double exactMass
                                , double matchedMass
                                , double time ) : formula_( formula )
                                                 , exactMass_( exactMass )
                                                 , matchedMass_( matchedMass )
                                                 , time_( time )
{
}

const std::string&
lockmass::reference::formula() const
{
    return formula_;
}

double
lockmass::reference::exactMass() const
{
    return exactMass_;
}

double
lockmass::reference::matchedMass() const
{
    return matchedMass_;
}

double
lockmass::reference::time() const
{
    return time_;
}

//////////////////////
void
lockmass::clear()
{
    references_.clear();
}

///////////////////

// static
bool
lockmass::findReferences( lockmass& lk,  const adcontrols::MassSpectrum& ms )
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
lockmass::findReferences( lockmass& lk,  const adcontrols::MassSpectrum& ms, int idx, int fcn )
{
    static ChemicalFormula formulaParser;

    if ( idx < 0 || fcn < 0 )
        return false;

    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( ms );
    if ( fcn >= segs.size() )
        return false;

    auto& annots = segs[ fcn ].get_annotations();

    auto it = std::find_if( annots.begin(), annots.end(), [=]( const adcontrols::annotation& a ){ return a.index() == idx; });
    if ( it != annots.end() ) {
        const std::string& formula = it->text();
        double exactMass = formulaParser.getMonoIsotopicMass( formula );
        double matchedMass = segs[ fcn ].getMass( it->index() );
        double time        = segs[ fcn ].getTime( it->index() );
        lk << lockmass::reference( formula, exactMass, matchedMass, time );        

        return true;
    }
    return false;
}

bool
lockmass::fit()
{
    fitter_.coeffs_.clear();
    
    if ( references_.size() == 1 ) {
        auto& ref = references_[0];
        double error = ref.matchedMass() - ref.exactMass();
        double relativeError = error / ref.matchedMass();

        fitter_.coeffs_.push_back( relativeError );
        return true;

    } else if ( references_.size() >= 2 ) {

        std::vector< double > x, y;

        for ( const auto& ref: references_ ) {
            double error = ref.matchedMass() - ref.exactMass();
            x.push_back( ref.matchedMass() );
            y.push_back( error );
        }
        return adportable::polfit::fit( x.data(), y.data(), x.size(), 2, fitter_.coeffs_ );
    }

    return false;
}

bool
lockmass::operator()( MassSpectrum& ms ) const
{
    return fitter_( ms );
}

bool
lockmass::fitter::operator()( MassSpectrum& ms ) const
{
    if ( coeffs_.empty() )
        return false;

    segment_wrapper<> segs( ms );

    for ( auto& fms: segs ) {
        const double * masses = fms.getMassArray();

        if ( coeffs_.size() == 1 ) {
            // relative error correction
            for ( size_t i = 0; i < fms.size(); ++i ) {
                double mass = masses[i] - masses[i] * coeffs_[0];
                fms.setMass( i, mass );
            }

        } else {

            for ( size_t i = 0; i < fms.size(); ++i ) {
				double mass = masses[i] - adportable::polfit::estimate_y( coeffs_, masses[ i ] );
                fms.setMass( i, mass );
            }

        }
    }

	return true;
}

void
lockmass::fitter::clear()
{
    coeffs_.clear();
}

