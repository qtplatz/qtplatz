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

#include "targeting.hpp"
#include "targetingmethod.hpp"
#include "chemicalformula.hpp"
#include "massspectrum.hpp"
#include "msproperty.hpp"
#include "samplinginfo.hpp"
#include <adcontrols/moltable.hpp>
#include <adportable/debug.hpp>
#include <adportable/combination.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <set>
#include <map>

using namespace adcontrols;

Targeting::Targeting() : method_( std::make_shared< TargetingMethod >() )
{
}

Targeting::Targeting( const Targeting& t ) : method_( t.method_ )
                                           , candidates_( t.candidates_ )
                                           , active_formula_( t.active_formula_ )
                                           , pos_adducts_( t.pos_adducts_ )
                                           , neg_adducts_( t.neg_adducts_ )
{
}

Targeting::Targeting( const TargetingMethod& m ) : method_( std::make_shared<TargetingMethod>( m ) )
{
    setup( m );
}

Targeting::Candidate::Candidate() : idx(0)
                                  , fcn(0)
                                  , charge(1)
                                  , mass_error(0)
{
}

Targeting::Candidate::Candidate( const Candidate& t ) : idx( t.idx )
                                                      , fcn( t.fcn )
                                                      , charge( t.charge )
                                                      , mass_error( t.mass_error )
                                                      , formula( t.formula )
{
}


Targeting::Candidate::Candidate( uint32_t _idx, uint32_t _fcn, uint32_t _charge, double _error, const std::string& _formula )
    : idx( _idx )
    , fcn( _fcn )
    , charge( _charge )
    , mass_error( _error )
    , formula( _formula )
{
}

bool
Targeting::find_candidate( const MassSpectrum& ms, int fcn, bool polarity_positive, const std::vector< charge_adduct_type >& list )
{
    if ( ms.size() == 0 )
        return false;

    adcontrols::MSFinder finder( method_->tolerance( method_->toleranceMethod() ), method_->findAlgorithm(), method_->toleranceMethod() );

    for ( auto& formula : active_formula_ ) {
        double target_mass = formula.second; // search 'M'

        size_t pos = finder( ms, target_mass );
        if ( pos != MassSpectrum::npos ) {
            double error = ms.getMass( pos ) - target_mass;
            candidates_.push_back( Candidate( uint32_t( pos /*idx*/ ), fcn, 1/*charge*/, error, formula.first /*formula*/ ) );
        }
    }

    (void)polarity_positive; // this will be necessary for account an electron mass, todo
    for ( auto& adduct : list ) {
        for ( auto& formula : active_formula_ ) {
            double target_mass = ( formula.second /* M */ + std::get<0>( adduct /*mass*/ ) ) / std::get<2>( adduct /*charge*/ );

            size_t pos = finder( ms, target_mass );
            if ( pos != MassSpectrum::npos ) {
                double error = ms.getMass( pos ) - target_mass;
                candidates_.push_back( Candidate( uint32_t( pos ), fcn, std::get<2>( adduct ), error, formula.first + "+" + std::get<1>( adduct ) ) );
            }
        }
    }
    return true;
}

bool
Targeting::operator()( const MassSpectrum& ms )
{
    candidates_.clear();

    if ( !ms.isCentroid() )
        return false;

    bool polarity_positive = (ms.polarity() == PolarityPositive) || (ms.polarity() == PolarityIndeterminate);

    segment_wrapper< const adcontrols::MassSpectrum > segs( ms );
    int fcn = 0;
    for ( auto& fms : segs ) {
        if ( polarity_positive ) {
            find_candidate( fms, fcn++, true, poslist_ );
        }
        else {
            find_candidate( fms, fcn++, false, neglist_ );
        }
    }
    return true;
}

bool
Targeting::force_find( const MassSpectrum& ms, const std::string& formula, int32_t fcn )
{
    candidates_.clear();

    if ( !ms.isCentroid() )
        return false;
    if ( ms.size() == 0 )
        return false;

    double mass = ChemicalFormula().getMonoIsotopicMass( formula );

    int proto( 0 );
    for ( auto& fms: segment_wrapper< const adcontrols::MassSpectrum >( ms ) ) {
        if ( proto == fcn ) {
            double width = method_->tolerance( idToleranceDaltons );
            do {
                adcontrols::MSFinder finder( width, idFindClosest );
                size_t pos = finder( fms, mass );
                if ( pos != MassSpectrum::npos ) {
                    candidates_.emplace_back( pos, proto, /*charge*/ 1, fms.getMass( pos ) - mass, formula );
                    return true;
                }
                width += width;
            } while ( width < 1.0 );
        }
        ++proto;
    }
    return false;
}

void
Targeting::setup( const TargetingMethod& m )
{
    ChemicalFormula formula_parser;

    active_formula_.clear();

    for ( auto& x : m.molecules().data() ) {
        if ( x.enable() ) {
            auto formula = std::string( x.formula() ) + x.adducts();
            active_formula_.push_back( std::make_pair( formula, formula_parser.getMonoIsotopicMass( ChemicalFormula::split( formula ) ) ) );
        }
    }
    setup_adducts( m, true, pos_adducts_ );
    std::sort( pos_adducts_.begin(), pos_adducts_.end() );

    setup_adducts( m, false, neg_adducts_ );
    std::sort( neg_adducts_.begin(), neg_adducts_.end() );

    auto charge_range = m.chargeState();
    
    for ( uint32_t charge = charge_range.first; charge <= charge_range.second; ++charge ) {
        make_combination( charge, pos_adducts_, poslist_ );
        make_combination( charge, neg_adducts_, neglist_ );
    }

    std::sort( poslist_.begin(), poslist_.end(), [] ( const charge_adduct_type& a, const charge_adduct_type& b ){
        return std::get<0>( a ) < std::get<0>( b ); // order by mass
    } );

    std::sort( neglist_.begin(), neglist_.end(), [] ( const charge_adduct_type& a, const charge_adduct_type& b ){
        return std::get<0>( a ) < std::get<0>( b ); // order by mass
    } );

}

void
Targeting::setup_adducts( const TargetingMethod& m, bool positive, std::vector< adduct_type >& adducts )
{
    ChemicalFormula formula_parser;

    for ( auto& a: m.adducts( positive ) ) { 

        if ( a.first ) { // if (enable)
            std::string addformula;
            std::string loseformula;
            std::pair< double, double > addlose( 0, 0 );
            if ( !addformula.empty() ) {
                addlose.first = formula_parser.getMonoIsotopicMass( addformula );
            }
            if ( !loseformula.empty() ) {
                addlose.second = -formula_parser.getMonoIsotopicMass( loseformula );
            }
            adducts.push_back( std::make_pair( addlose.first + addlose.second, a.second ) );
        }
    }
}

void
Targeting::make_combination( uint32_t charge
                             , const std::vector< adduct_type >& adducts
                             , std::vector< charge_adduct_type >& list )
{
    if ( adducts.empty() || charge == 0 )
        return;

    typedef size_t formula_number;
    typedef size_t formula_count;
    std::set< std::map< formula_number, formula_count > > combination_with_repetition;
    {
        // adportable::scoped_debug<> scope( __FILE__, __LINE__ ); scope << "making combination(2):";
        std::vector< uint32_t > selector( adducts.size() * charge );
        auto it = selector.begin();
        for( uint32_t n = 0; n < adducts.size(); ++n ) {
            std::fill( it, it + charge, n );
            std::advance( it, charge );
        }
        do {
            std::map< formula_number, formula_count > formulae;

            for ( auto it = selector.begin(); it != selector.begin() + charge; ++it )
                formulae[ *it ]++;

            combination_with_repetition.insert( formulae );

        } while ( boost::next_combination( selector.begin(), selector.begin() + charge, selector.end() ) );
    }

    // translate index combination into a vector of atoms|formula with exact-mass

    for ( auto& fmap : combination_with_repetition ) {
        std::ostringstream formula;
        double mass = 0;
        std::for_each( fmap.begin(), fmap.end(), [&] ( const std::pair< size_t, size_t >& a ){
            formula << a.second << "(" << adducts[ a.first ].second << ")";
            mass += adducts[ a.first ].first * a.second;
        } );
        list.push_back( std::make_tuple( mass, formula.str(), charge ) );
        // ADDEBUG() << "charge: " << charge << "\tmass=" << mass << "\t" << formula.str();
    }
}

bool
Targeting::archive( std::ostream& os, const Targeting& v )
{
    portable_binary_oarchive ar( os );
    ar << v;
    return true;
}

bool
Targeting::restore( std::istream& is, Targeting& v )
{
    portable_binary_iarchive ar( is );
    ar >> v;
    return true;
}
