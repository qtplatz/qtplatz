// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include <compiler/diagnostic_push.h>
#include <compiler/workaround.h>
#include <compiler/disable_unused_parameter.h>

#include "chemicalformula.hpp"
#include "tableofelements.hpp"
#include "ctable.hpp"
#include "element.hpp"
#include <adportable/string.hpp>
#include <boost/noncopyable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <compiler/diagnostic_pop.h>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using namespace adcontrols;


namespace adcontrols {

    namespace client {

        namespace qi = boost::spirit::qi;

        const wchar_t * element_table [] = {
            L"H",                                                                                                                  L"He",
            L"Li", L"Be",                                                                       L"B",  L"C",  L"N",  L"O",  L"F",  L"Ne", 
            L"Na", L"Mg",                                                                       L"Al", L"Si", L"P",  L"S",  L"Cl", L"Ar",
            L"K",  L"Ca", L"Sc", L"Ti", L"V",  L"Cr", L"Mn", L"Fe", L"Co", L"Ni", L"Cu", L"Zn", L"Ga", L"Ge", L"As", L"Se", L"Br", L"Kr",  
            L"Rb", L"Sr", L"Y",  L"Zr", L"Nb", L"Mo", L"Tc", L"Ru", L"Rh", L"Pd", L"Ag", L"Cd", L"In", L"Sn", L"Sb", L"Te", L"I",  L"Xe",  
            L"Cs", L"Ba", L"Lu", L"Hf", L"Ta", L"W",  L"Re", L"Os", L"Ir", L"Pt", L"Au", L"Hg", L"Tl", L"Pb", L"Bi", L"Po", L"At", L"Rn",
            L"Fr", L"Ra", L"Ac", L"Th", L"Pa", L"U",  L"Np", L"Pu", L"Am", L"Cm", L"Bk", L"Cf", L"Es", L"Fm", L"Md", L"No", L"Lr",
            // Lanthanoids
            L"La", L"Ce", L"Pr", L"Nd", L"Pm", L"Sm", L"Eu", L"Gd", L"Tb", L"Dy",  L"Ho", L"Er" L"Tm", L"Yb",
            // Actinoids
            L"Ac", L"Th", L"Pa", L"U", L"Np", L"Pu", L"Am", L"Cm", L"Bk", L"Cf", L"Es", L"Fm", L"Md", L"No"
        };

        typedef std::map< const wchar_t *, std::size_t > map_type;

        void map_add( map_type& m, const std::pair<const wchar_t *, std::size_t>& p ) {
            m[ p.first ] += p.second;
        }

        void map_join( map_type& m, map_type& a ) {
            BOOST_FOREACH( map_type::value_type& p, a )
                m[ p.first ] += p.second;
        }

        void map_mul( map_type& m, std::size_t n ) {
            BOOST_FOREACH( map_type::value_type& p, m )
                p.second *= n;
        }

#if ! defined __APPLE__

        template<typename Iterator>
        struct chemical_formula_parser : boost::spirit::qi::grammar< Iterator, map_type() > {

            chemical_formula_parser() : chemical_formula_parser::base_type( molecule ), element( element_table, element_table )  {
                using boost::phoenix::bind;
                using boost::spirit::qi::_val;
                using boost::spirit::ascii::space;

                molecule =
                         +(
                              atoms          [ boost::phoenix::bind(&map_add, _val, qi::_1) ]
                            | repeated_group [ boost::phoenix::bind(&map_join, _val, qi::_1 ) ] 
                            | space
                          )
                          ;
                atoms = 
                          element >> ( qi::uint_ | qi::attr(1u) ) // default to 1
                          ;
                repeated_group %= // forces attr proparation
                          '(' >> molecule >> ')'
                          >> qi::omit[ qi::uint_[ boost::phoenix::bind( map_mul, qi::_val, qi::_1 ) ] ]
                          ;
            }
            qi::rule<Iterator, std::pair< const wchar_t *, std::size_t >() > atoms;
            qi::rule<Iterator, map_type()> molecule, repeated_group;
            qi::symbols<wchar_t, const wchar_t *> element;
        };
#endif
    }

    namespace internal {
        class ChemicalFormulaImpl {
        public:
            ChemicalFormulaImpl();
            double getMonoIsotopicMass( const std::wstring& formula );
            double getChemicalMass( const std::wstring& formula );
            std::wstring standardFormula( const std::wstring& formula );

			static bool parse( const std::wstring& formula, client::map_type& map );
        };
    }

}

ChemicalFormula::~ChemicalFormula(void)
{
    delete impl_;
}

ChemicalFormula::ChemicalFormula() : impl_( new internal::ChemicalFormulaImpl )
{
}

double
ChemicalFormula::getMonoIsotopicMass( const std::wstring& formula )
{
    return impl_->getMonoIsotopicMass( formula );
}

double
ChemicalFormula::getChemicalMass( const std::wstring& formula )
{
    return impl_->getChemicalMass( formula );
}

std::wstring
ChemicalFormula::standardFormula( const std::wstring& formula )
{
    return impl_->standardFormula( formula );
}

namespace adcontrols {

	//struct is_connect {
	size_t bond_connect( const CTable::Bond& bond, size_t num ) {
		return ( size_t( bond.first_atom_number ) == num ) ? bond.second_atom_number
			: ( size_t( bond.second_atom_number ) == num ) ? bond.first_atom_number : 0;
	}
}

std::wstring
ChemicalFormula::getFormula( const CTable& ctable )
{
	using adcontrols::CTable;

	adcontrols::TableOfElements * toe = adcontrols::TableOfElements::instance();

	typedef std::pair< std::wstring, int > atom_valence_t;
	std::vector< atom_valence_t > valences;

	BOOST_FOREACH( const CTable::Atom& atom, ctable.atoms() ) {
		const adcontrols::Element& element = toe->findElement( atom.symbol );
		assert( ! element.symbol().empty() );
		valences.push_back( std::make_pair<std::wstring, int>( std::wstring( atom.symbol ), element.valence() ) );
	}

	BOOST_FOREACH( const CTable::Bond& bond, ctable.bonds() ) {
		size_t n = bond.bond_type <= 3 ? bond.bond_type : 0;
		valences[ bond.first_atom_number - 1 ].second -= n;
		valences[ bond.second_atom_number - 1 ].second -= n;
	}

	do {
		std::vector< atom_valence_t >::iterator atomIt = valences.begin(); 
		do {
			// find atom that has negataive valence value
			atomIt = std::find_if( atomIt, valences.end(), boost::bind( &atom_valence_t::second, _1) < 0 );
			if ( atomIt != valences.end() ) {
				size_t atom_number = std::distance( valences.begin(), atomIt ) + 1;

				CTable::bond_vector::const_iterator bondIt = ctable.bonds().begin();
				while ( bondIt != ctable.bonds().end() ) {
					bondIt = std::find_if( bondIt, ctable.bonds().end(), boost::bind( &bond_connect, _1, atom_number ) );
					if ( bondIt != ctable.bonds().end() ) {
						size_t adjacent_number = bond_connect( *bondIt, atom_number );
						while ( valences[ adjacent_number - 1 ].second > 0 && atomIt->second < 0 ) {
							valences[ adjacent_number - 1 ].second--;
							atomIt->second++;
						}
						++bondIt;
					}
				}
				++atomIt;
			}
		} while ( atomIt != valences.end() );
	} while(0);
	std::wostringstream formula;
	for ( size_t i = 0; i < valences.size(); ++i ) {
		const std::pair< std::wstring, int >& v = valences[ i ];
		formula << v.first;

		if ( v.second >= 1 ) {
			formula << L"H";
			if ( v.second >= 2 )
				formula << v.second;
		}
		formula << L" ";
	}
	return formula.str();
}

std::map< const wchar_t *, size_t >
ChemicalFormula::getComposition( const std::wstring& formula )
{
	using internal::ChemicalFormulaImpl;

	std::map< const wchar_t *, size_t > comp;
    client::map_type map;
	if ( ChemicalFormulaImpl::parse( formula, map ) ) {
		BOOST_FOREACH( client::map_type::value_type& p, map )
			comp[ p.first ] = p.second;
	}
    return comp;
}

///////////////
using namespace adcontrols::internal;

ChemicalFormulaImpl::ChemicalFormulaImpl() 
{
}

double
ChemicalFormulaImpl::getChemicalMass( const std::wstring& formula )
{
    client::map_type map;
    if ( parse( formula, map ) ) {
        adcontrols::TableOfElements *toe = adcontrols::TableOfElements::instance();
        double mass = 0;
        BOOST_FOREACH( client::map_type::value_type& p, map )
            mass += toe->getChemicalMass( toe->findElement( p.first ) ) * p.second;
        return mass;
    }
    return 0;
}

double
ChemicalFormulaImpl::getMonoIsotopicMass( const std::wstring& formula )
{
    client::map_type map;
    if ( parse( formula, map ) ) {
        adcontrols::TableOfElements *toe = adcontrols::TableOfElements::instance();
        double mass = 0;
        BOOST_FOREACH( client::map_type::value_type& p, map )
            mass += toe->getMonoIsotopicMass( toe->findElement( p.first ) ) * p.second;
        return mass;
    }
    return 0;
}

std::wstring
ChemicalFormulaImpl::standardFormula( const std::wstring& formula )
{
    client::map_type map;
    if ( parse( formula, map ) ) {

        std::wostringstream o;
        BOOST_FOREACH( client::map_type::value_type& p, map )
            o << p.first << p.second;
        return o.str();
    }
    return L"";
}

bool
ChemicalFormulaImpl::parse( const std::wstring& formula, client::map_type& map )
{
    typedef std::wstring::const_iterator iterator_type;
#if ! defined __APPLE__
    client::chemical_formula_parser< iterator_type > cf;
    iterator_type it = formula.begin();
    iterator_type end = formula.end();
    return boost::spirit::qi::parse( it, end, cf, map ) && it == end;
#else
    (void)formula;
    (void)map;
    return false;
#endif
}
