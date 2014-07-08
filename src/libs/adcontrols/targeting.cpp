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
#include <adportable/debug.hpp>
#include <algorithm>
#include <sstream>

using namespace adcontrols;

Targeting::Targeting()
{
}

Targeting::Targeting( const Targeting& t ) : candidates_( t.candidates_ )
                                           , active_formula_( t.active_formula_ )
                                           , pos_adducts_( t.pos_adducts_ )
                                           , neg_adducts_( t.neg_adducts_ )
{
}

Targeting::Targeting( const TargetingMethod& m )
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

bool
Targeting::operator()( const MassSpectrum& )
{
    ADDEBUG() << "Targeting operator...";

    candidates_.clear();
    return true;
}

void
Targeting::setup( const TargetingMethod& m )
{
    ChemicalFormula formula_parser;

    ADDEBUG() << "setup start";

    active_formula_.clear();
    for ( auto& f: m.formulae() ) {
        if ( TargetingMethod::formula_data::enable( f ) ) {
            const std::string formula = TargetingMethod::formula_data::formula( f );
            active_formula_.push_back( std::make_pair( formula, formula_parser.getMonoIsotopicMass( formula ) ) );
        }
    }
    setup_adducts( m, true, pos_adducts_ );
    std::sort( pos_adducts_.begin(), pos_adducts_.end() );

    setup_adducts( m, false, neg_adducts_ );
    std::sort( neg_adducts_.begin(), neg_adducts_.end() );

    auto charge_range = m.chargeState();
    
    for ( uint32_t charge = charge_range.first; charge <= charge_range.second; ++charge ) {
        make_combination( charge, pos_adducts_, poslist_ );
        // make_combination( charge, neg_adducts_, neglist_ );
    }

    ADDEBUG() << "setup completed";
}

void
Targeting::setup_adducts( const TargetingMethod& m, bool positive, std::vector< adduct_type >& adducts )
{
    ChemicalFormula formula_parser;

    for ( auto& a: m.adducts( positive ) ) { 

        if ( a.first ) { // if (enable)
            std::string addformula;
            std::string loseformula;

            std::vector< std::string > formulae;
            if ( formula_parser.split( a.second, formulae ) ) {
                for ( auto& tok: formulae ) {
                    bool addlose( true );
                    if ( tok == "+" ) {
                        addlose = true;
                    } else if ( tok == "-" ) {
                        addlose = false;
                    } else {
                        std::string& t = addlose ? addformula : loseformula;
                        t += tok;
                    }                        
                }
            }
            double adduct_mass = formula_parser.getMonoIsotopicMass( addformula );
            double lose_mass = formula_parser.getMonoIsotopicMass( loseformula );

            adducts.push_back( std::make_pair( adduct_mass - lose_mass, a.second ) );
        }
    }
}

void
Targeting::make_combination( uint32_t charge
                             , const std::vector< adduct_type >& adducts
                             , std::vector< charge_adduct_type >& list )
{
    ADDEBUG() << "make_combination charge=" << charge;

    if ( charge == 1 ) {
        for ( auto& a: adducts )
            list.push_back( std::make_tuple( a.first, a.second, charge ) );
        return;
    }

    std::vector< bool > selector( adducts.size() );
    std::fill( selector.begin() + charge, selector.end(), true );

    do {

        double adduct_mass = 0;
        std::ostringstream o;

        for ( int i = 0; i < int(adducts.size()); ++i ) {
            if ( ! selector[i] ) {
                adduct_mass += adducts[ i ].first;
                o << "(" << adducts[ i ].second << ")";                
            }
        }
        list.push_back( std::make_tuple( adduct_mass, o.str(), charge ) );

        ADDEBUG() << o.str();

    } while ( std::next_permutation( selector.begin(), selector.end() ) );

}
