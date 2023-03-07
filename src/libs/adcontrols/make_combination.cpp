/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "make_combination.hpp"
#include "constants.hpp"
#include "chemicalformula.hpp"
#include "ionreactionmethod.hpp"
#include "molecule.hpp"
#include <adportable/combination.hpp>
#include <adportable/debug.hpp>
#include <adportable/for_each_combination.hpp>
#include <boost/format.hpp>
#include <map>
#include <iterator>

namespace {

    typedef std::map< std::string, uint32_t > adduct_repeat_t; // formula, repeat
    using namespace adcontrols;

    std::string
    make_addlose_string( std::pair< mol::molecule, mol::molecule >&& addlose, int scharge ) {
        std::ostringstream o;
        if ( !std::get< 0 >( addlose ).elements().empty() ) { // has adduct
            const auto& add = std::get< 0 >( addlose );
            if ( std::get< 1 >( addlose ).elements().empty() ) // without loss
                o << "+[" << add.formula( false ) << "]";
            else
                o << "+" << add.formula( false );
        }

        if ( !std::get< 1 >( addlose ).elements().empty() ) { // has loss
            const auto& loss = std::get< 1 >( addlose );
            if ( (scharge < 0 ) && ( loss.elements().size() == 1 && loss.elements().at( 0 ).atomicNumber() == 1 ) ) { // H
                // -H --> -[H]+ to handle polarity appropriately in ChemicalFormula class
                scharge = (-scharge);
            }
            o << "-[" << loss.formula( false ) << "]";
        }
        if ( std::abs( scharge ) > 1 )
            o << std::abs( scharge );
        o << (scharge < 0 ? '-' : '+');
        return o.str();
    }

    std::string to_string( const adduct_repeat_t& alist ) {
        std::ostringstream o;
        for ( const auto& [formula, repeat]: alist ) {
            if ( ! o.str().empty() )
                o << "; ";
            if ( repeat == 1 )
                o << formula;
            else
                o << "(" << formula << ")" << repeat;
        }
        return o.str();
    }
    // std::string to_string( const std::vector< adcontrols::ChemicalFormula::formula_adduct_t >& v ) {
    //     std::ostringstream o;
    //     for ( const auto& t: v )
    //         o << "{" << t.first << ", " << t.second << "}";
    //     return o.str();
    // }
    ///////////////////////////////////////////////////////////////////
}


namespace adcontrols {
    namespace lipidid {

        std::pair< int, std::string >
        to_string( const std::pair< adcontrols::mol::molecule, adcontrols::mol::molecule >& addsub ) {
            const auto& [add,sub] = addsub;
            int charge = add.charge() + sub.charge() * -1;
            std::ostringstream o;
            if ( add ) {
                o << "+" << add.formula( true );
            }
            if ( sub ) {
                if ( ! o.str().empty() )
                    o << " ";
                o << "-" << sub.formula( true );
            }
            return { charge, o.str() };
        }

        std::pair< adcontrols::mol::molecule, adcontrols::mol::molecule >
        merge_molecule( const std::vector< molecule_pair_t >& alist ) {
            adcontrols::mol::molecule add, sub;
            for ( const auto& u: alist ) {
                const size_t repeat = std::get< 3 >( u );
                add += std::get< 0 >( u ) * ( std::get< 0 >( u ).charge() ? repeat : 1 );
                sub += std::get< 1 >( u ) * ( std::get< 1 >( u ).charge() ? repeat : 1 );
            }
            return { add, sub };
        }

        std::vector< std::vector< molecule_pair_t > >
        make_combination( uint32_t charge, const std::vector< std::string >& adducts, adcontrols::ion_polarity pol )
        {
            const size_t r = charge;
            const size_t n = adducts.size();
            if ( r == 0 || n == 0 )
                return {};

            // Compute cartesian products
            std::vector< std::map< std::string, size_t > > cartesian_products;

            std::vector< std::vector< std::string >::const_iterator > v_iter( r, adducts.begin() );
            std::map< std::string, size_t > products;
            do {
                std::for_each( v_iter.begin(), v_iter.end(), [&]( const auto& it ){
                    products[ *it ]++;
                });
                cartesian_products.emplace_back( std::move( products ) );
            } while ( boost::next_mapping( v_iter.begin(), v_iter.end(), adducts.begin(), adducts.end() ) );
            // <--- End compute cartesian products

            // --- original formula string to mol list and repeat
            std::vector< std::vector< molecule_pair_t > > mlist;
            for ( const auto& amap: cartesian_products ) {
                // following loop correspoinding to a set of adduct/lose
                std::vector< molecule_pair_t > alist;
                for ( const auto& [formula,repeat]: amap ) {
                    molecule_pair_t addlose{ {}, {}, formula, repeat }; // set original formula as a key
                    auto v = adcontrols::ChemicalFormula::split( formula ); // formula_adduct_t := std::pair< std::string, char (leading +/-) >
                    for ( auto a: v ) {
                        if ( a.second == '-' ) {
                            std::get< 1 >( addlose ) += ChemicalFormula::toMolecule( a.first ); // sub (ex. -H, -OH)
                        } else if ( a.second == '+' || a.second == ' ' ) {
                            std::get< 0 >( addlose ) += ChemicalFormula::toMolecule( a.first ); // add  (ex. +H, +Na)
                        }
                    }
                    // merge( addlose );
                    alist.emplace_back( std::move( addlose ) );
                }
                mlist.emplace_back( std::move( alist ) );
            }
            return mlist;
        }

        //////////////

        // std::vector< std::pair< int, std::string > > // charge, addlose
        std::vector< std::vector< molecule_pair_t > >
        make_combination( const adcontrols::IonReactionMethod& t, ion_polarity pol )
        {
            // extract enabled formula
            std::vector< std::string > list;
            for( const auto& [enable, formula]: t.addlose( pol ) ) {
                if ( enable )
                    list.emplace_back( formula );
            }

            // std::vector< std::pair< int, std::string > > res;
            std::vector< std::vector< molecule_pair_t > > mlist;
            const auto& range = t.chargeState( pol );
            for ( uint32_t charge = range.first; charge <= range.second; ++charge ) {
                // int scharge = ( pol == adcontrols::polarity_positive ? static_cast< int >( charge ) : (-static_cast< int >(charge)) );
                auto alist = make_combination( charge, list, pol );
                mlist.insert( mlist.end()
                              , std::make_move_iterator( alist.begin() )
                              , std::make_move_iterator( alist.end() ) );
                // res.insert( res.end()
                //             , std::make_move_iterator( alist.begin() )
                //             , std::make_move_iterator( alist.end() ) );
            }
            return mlist;
            // return res;
        }

        //////////
        std::pair< int, std::string >
        to_string( const std::vector< molecule_pair_t >& alist ) {
            return to_string( merge_molecule( alist ) );
        }

    }
}
