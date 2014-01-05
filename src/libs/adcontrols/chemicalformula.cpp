// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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
#include <adportable/utf.hpp>
#include <boost/noncopyable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <compiler/diagnostic_pop.h>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#if defined _MSC_VER
# pragma warning( disable: 4503)
#endif
using namespace adcontrols;


namespace adcontrols {

    namespace client {

        using boost::phoenix::bind;
        using boost::spirit::qi::_val;
        using boost::spirit::qi::_1;
        using boost::spirit::ascii::space;
        
        namespace qi = boost::spirit::qi;

        const char * element_table [] = {
            "H",                                                                                                              "He",
            "Li", "Be",                                                                         "B",  "C",  "N",  "O",  "F",  "Ne", 
            "Na", "Mg",                                                                         "Al", "Si", "P",  "S",  "Cl", "Ar",
            "K",  "Ca", "Sc", "Ti", "V",  "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn", "Ga", "Ge", "As", "Se", "Br", "Kr",  
            "Rb", "Sr", "Y",  "Zr", "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn", "Sb", "Te", "I",  "Xe",  
            "Cs", "Ba", "Lu", "Hf", "Ta", "W",  "Re", "Os", "Ir", "Pt", "Au", "Hg", "Tl", "Pb", "Bi", "Po", "At", "Rn",
            "Fr", "Ra", "Ac", "Th", "Pa", "U",  "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm", "Md", "No", "Lr",
            // Lanthanoids
            "La", "Ce", "Pr", "Nd", "Pm", "Sm",  "Eu", "Gd",  "Tb", "Dy",  "Ho", "Er" "Tm", "Yb",
            // Actinoids
            "Ac", "Th", "Pa", "U", "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm", "Md", "No"
        };

        typedef std::pair< size_t, const char * > atom_type;

        // for chemical composition
        typedef std::map< atom_type, size_t > map_type;
        struct formulaComposition {
            static void formula_add( map_type& m, const std::pair<const atom_type, std::size_t>& p ) {
                m[ p.first ] += p.second;
            }
            
            static void formula_join( map_type& m, map_type& a ) {
                for( map_type::value_type& p: a )
                    m[ p.first ] += p.second;
            }
            
            static void formula_repeat( map_type& m, std::size_t n ) {
                for( map_type::value_type& p: m )
                    p.second *= n;
            }
        };

        // for chemical formula formatter
        static const char * braces [] = { "(", ")" };

        typedef std::vector< std::pair< atom_type, size_t > > format_type;
        struct formulaFormat {
            static void formula_add( format_type& m, const std::pair<const atom_type, std::size_t>& p ) {
                m.push_back( p );
            }
            static void formula_join( format_type& m, format_type& a ) {
                m.push_back( std::make_pair( atom_type( 0, braces[0] ), 0 ) );
                for ( auto t: a )
                    m.push_back( t );
            }
            static void formula_repeat( format_type& m, std::size_t n ) {
                m.push_back( std::make_pair( atom_type( 0, braces[1] ), n ) );
            }
        };

        template<typename Iterator, typename handler, typename startType>
        struct chemical_formula_parser : boost::spirit::qi::grammar< Iterator, startType() > {
            
            chemical_formula_parser() : chemical_formula_parser::base_type( molecule ), element( element_table, element_table )  {
                molecule =
                    + (
                        atoms            [ boost::phoenix::bind(&handler::formula_add, _val, qi::_1) ]
                        | repeated_group [ boost::phoenix::bind(&handler::formula_join, _val, qi::_1 ) ]
                        | space
                        )
                    ;
                atoms = 
                    atom >> ( qi::uint_ | qi::attr(1u) ) // default to 1
                    ;
                atom =
                    ( qi::uint_ | qi::attr(0u) ) >> element
                    ;
                repeated_group %= // forces attr proparation
                    '(' >> molecule >> ')'
                        >> qi::omit[ qi::uint_[ boost::phoenix::bind( handler::formula_repeat, qi::_val, qi::_1 ) ] ]
                    ;
            }
            
            qi::rule<Iterator, atom_type() > atom;
            qi::rule<Iterator, std::pair< atom_type, std::size_t >() > atoms;
            qi::rule<Iterator, startType()> molecule, repeated_group;
            qi::symbols<char, const char *> element;
        };

    }

    namespace internal {

        class ChemicalFormulaImpl {
        public:
            ChemicalFormulaImpl();

            template< typename char_type > static double getMonoIsotopicMass( const std::basic_string< char_type >& formula ) {
                client::map_type map;
                if ( parse( formula, map ) ) {
                    adcontrols::TableOfElements *toe = adcontrols::TableOfElements::instance();
                    double mass = 0;
                    for ( client::map_type::value_type& p: map ) {
                        const char * element_name = p.first.second;
                        size_t isotope = p.first.first;
                        mass += toe->getMonoIsotopicMass( toe->findElement( element_name ), isotope ) * p.second;
                    }
                    return mass;
                }
                return 0;
            }

            template< typename char_type > static double getChemicalMass( const std::basic_string< char_type >& formula ) {
                client::map_type map;
                if ( parse( formula, map ) ) {
                    adcontrols::TableOfElements *toe = adcontrols::TableOfElements::instance();
                    double mass = 0;
                    for ( client::map_type::value_type& p: map )
                        mass += toe->getChemicalMass( toe->findElement( p.first.second ) ) * p.second;
                    return mass;
                }
                return 0;
            }

			template< typename char_type > static bool parse( const std::basic_string< char_type >& formula, client::map_type& map ) {
                
                typedef typename std::basic_string< char_type >::const_iterator iterator_type;
                
                client::chemical_formula_parser< iterator_type, client::formulaComposition, client::map_type > cf;
                iterator_type it = formula.begin();
                iterator_type end = formula.end();

                return boost::spirit::qi::parse( it, end, cf, map ) && it == end;
            }

			template< typename char_type > static bool format( const std::basic_string< char_type >& formula, client::format_type& fmt ) {
                
                typedef typename std::basic_string< char_type >::const_iterator iterator_type;
                
                client::chemical_formula_parser< iterator_type, client::formulaFormat, client::format_type > cf;
                iterator_type it = formula.begin();
                iterator_type end = formula.end();

                return boost::spirit::qi::parse( it, end, cf, fmt ) && it == end;
            }

            template< typename char_type > static std::basic_string<char_type> standardFormula( const std::basic_string<char_type>& formula ) {
                client::map_type map;
                if ( parse( formula, map ) ) {
                    std::basic_ostringstream<char_type> o;

                    for ( client::map_type::value_type& p: map ) {
						std::basic_string<char_type> atom( p.first.second, p.first.second + std::strlen( p.first.second ) );
                        if ( p.first.first == 0 ) {
                            o << atom; // adportable::utf::to_wstring( p.first.second );
                            if ( p.second > 1 )  // omit '1' such as CH4, not C1H4
                                o << p.second;
                        } else {
                            o << atom; // p.first.first << adportable::utf::to_wstring( p.first.second );
                            if ( p.second > 1 ) 
                                o << p.second;
                            o << ' ';
                        }
                    }
                    return o.str();
                }
                return std::basic_string<char_type>();
            }
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
    return internal::ChemicalFormulaImpl::getMonoIsotopicMass( formula );
}

double
ChemicalFormula::getMonoIsotopicMass( const std::string& formula )
{
    return internal::ChemicalFormulaImpl::getMonoIsotopicMass( formula );
}

double
ChemicalFormula::getChemicalMass( const std::wstring& formula )
{
    return internal::ChemicalFormulaImpl::getChemicalMass( formula );
}

std::wstring
ChemicalFormula::standardFormula( const std::wstring& formula )
{
    return internal::ChemicalFormulaImpl::standardFormula( formula );
}

std::string
ChemicalFormula::standardFormula( const std::string& formula )
{
    return internal::ChemicalFormulaImpl::standardFormula( formula );
}

std::string
ChemicalFormula::formatFormula( const std::string& formula, bool richText )
{
    std::wstring wformula = formatFormula( adportable::utf::to_wstring( formula ), richText );
    return adportable::utf::to_utf8( wformula );
}

std::wstring
ChemicalFormula::formatFormula( const std::wstring& formula, bool richText )
{
    using adcontrols::internal::ChemicalFormulaImpl;
    client::format_type fmt;

    if ( ChemicalFormulaImpl::format( formula, fmt ) ) {

        std::wostringstream o;
        if ( richText ) {
            for ( auto e: fmt ) {
                if ( std::strcmp( e.first.second, "(" ) == 0 ) {
                    o << L"(";
                } else if ( std::strcmp( e.first.second, ")" ) == 0 ) {
                    o << boost::wformat( L")<sub>%1%</sub>" ) % e.second;
                } else {
                    if ( e.first.first )
                        o << boost::wformat( L"<sup>%1%</sup>" ) % e.first.first;
                    o << adportable::utf::to_wstring( e.first.second ); // element name
                    if ( e.second > 1 )
                        o << boost::wformat( L"<sub>%1%</sub>" ) % e.second;
                }
            }
        } else {
            bool sol = true;
            for ( auto e: fmt ) {
                if ( std::strcmp( e.first.second, "(" ) == 0 ) {
                    o << L"(";
                    sol = true;
                } else if ( std::strcmp( e.first.second, ")" ) == 0 ) {
                    o << e.second;
                } else {
                    if ( e.first.first ) {
                        if ( !sol )
                            o << L" " << e.first.first;
                        else
                            o << e.first.first;
                    }
                    o << adportable::utf::to_wstring( e.first.second ); // element name
                    if ( e.second > 1 )
                        o << e.second;
                    sol = false;
                }
            }
        }
        return o.str();
    }
    return L"";    
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

	for ( const CTable::Atom& atom: ctable.atoms() ) {
		const adcontrols::Element& element = toe->findElement( atom.symbol );
		assert( ! element.symbol().empty() );
		valences.push_back( std::make_pair<std::wstring, int>( std::wstring( atom.symbol ), element.valence() ) );
	}

	for ( const CTable::Bond& bond: ctable.bonds() ) {
		int n = bond.bond_type <= 3 ? bond.bond_type : 0;
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

std::map< std::string, size_t >
ChemicalFormula::getComposition( const std::wstring& formula )
{
	using internal::ChemicalFormulaImpl;

	std::map< std::string, size_t > comp;
    client::map_type map;
	if ( ChemicalFormulaImpl::parse( formula, map ) ) {
		for ( client::map_type::value_type& p: map ) {
            // size_t atomic_number = p.first.first;
            std::string element = p.first.second;

            // marge isotopes if specified in formula
            if ( comp.find( element ) == comp.end() )
                comp[ element ] = p.second;
            else
                comp[ element ] += p.second;
        }
	}
    return comp;
}

///////////////
using namespace adcontrols::internal;

ChemicalFormulaImpl::ChemicalFormulaImpl() 
{
}


