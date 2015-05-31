/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "quantarget.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <array>

using namespace quan;

static double mass_width = 0.002;
static double mass_offsets[] = { -mass_width * 3, -mass_width * 2, -mass_width * 1, 0.0, mass_width * 1, mass_width * 2, mass_width * 3 };

QuanTarget::QuanTarget( const std::string& formula
                        , bool positive_polarity
                        , const std::pair< int, int >& charge_range
                        , const std::vector< std::string >& adducts_losses ) : formula_( formula )
                                                                             , positive_polarity_( positive_polarity )
                                                                             , charge_range_( charge_range )
                                                                             , adducts_( adducts_losses )
{
    adcontrols::ChemicalFormula parser;
    
    for ( int charge = charge_range.first; charge <= charge_range.second; ++ charge ) {
        
        // todo: account electron mass depend on positive|negative
        if ( adducts_.empty() )
            formulae_.push_back( std::make_tuple( formula, parser.getMonoIsotopicMass( formula ) / double(charge), charge ) );

        for ( auto& adducts : adducts_ ) {
            std::string formula_adducts = formula + adducts;
            formulae_.push_back( std::make_tuple( formula_adducts, parser.getMonoIsotopicMass( formula_adducts ) / double( charge ), charge ) );
        }
        
    }
}

const std::string&
QuanTarget::formula() const
{
    return formula_;
}

const std::vector< QuanTarget::value_type> &
QuanTarget::formulae() const
{
    return formulae_;
}

void
QuanTarget::compute_candidate_masses( double mspeak_width, double tolerance, std::vector< computed_candidate_value >& masses )
{
    for ( auto& f: formulae_ ) {

        std::string formula_adducts = QuanTarget::formula( f );
        double exactMass = QuanTarget::exactMass( f );
        int charge = QuanTarget::charge( f );

        masses.push_back( std::make_tuple( formula_adducts, exactMass, charge, exactMass, mspeak_width ) );

        double d = mspeak_width;
        
        while ( d < tolerance ) {
            masses.push_back( std::make_tuple( formula_adducts, exactMass, charge, exactMass - d, mspeak_width ) );
            masses.push_back( std::make_tuple( formula_adducts, exactMass, charge, exactMass + d, mspeak_width ) );
            d += mspeak_width;
        }
    }

}
