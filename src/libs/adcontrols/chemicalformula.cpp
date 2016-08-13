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

#include <compiler/diagnostic_push.h>
#include <compiler/workaround.h>
#include <compiler/disable_unused_parameter.h>
#include "chemicalformula.hpp"
#include "tableofelement.hpp"
#include "ctable.hpp"
#include "element.hpp"
#include <adportable/utf.hpp>
#include <adportable/formula_parser.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/noncopyable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/tokenizer.hpp>
#include <compiler/diagnostic_pop.h>

#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <numeric>

#if defined _MSC_VER
# pragma warning( disable: 4503)
#endif
using namespace adcontrols;

namespace adcontrols {

    namespace chem {
        
        static const char * braces [] = { "(", ")" };
        static const char * separators [] = { "+", "-" };
        using adportable::chem::atom_type;
        
        typedef std::vector< std::pair< atom_type, size_t > > format_type;
        typedef std::pair< format_type, int > iformat_type;

        struct formulaFormat {
            static void formula_add( iformat_type& m, const std::pair<const atom_type, std::size_t>& p ) {
                m.first.emplace_back( p );
            }
            static void formula_join( iformat_type& m, iformat_type& a ) {
                if ( a.second )
                    m.second += a.second; // charge-group
                else
                    m.first.emplace_back( atom_type( 0, braces[0] ), 0 ); // repeat-group
                for ( auto t: a.first )
                    m.first.emplace_back( t );
                
            }
            static void formula_repeat( iformat_type& m, std::size_t n ) {
                m.first.emplace_back( atom_type( 0, braces[1] ), n );
            }
            static void charge_state( iformat_type& m, adportable::chem::charge_type& c ) {
                m.second += ( c.second == '+' ? c.first : -c.first );
            }
        };

        struct formatter {

            template< typename char_type > bool
            parse( typename std::basic_string< char_type >::const_iterator& it
                   , typename std::basic_string< char_type >::const_iterator end, iformat_type& fmt ) const {
                
                typedef typename std::basic_string< char_type >::const_iterator iterator_type;
                
                using namespace adportable::chem;
                adportable::chem::chemical_formula_parser< iterator_type, formulaFormat, iformat_type > format_parser;
                
                return boost::spirit::qi::parse( it, end, format_parser, fmt );
            }
            
            template<typename char_type> bool formulae( iformat_type& fmt, const std::basic_string<char_type>& formula ) const {
                
                typename std::basic_string< char_type >::const_iterator it = formula.begin();

                while( parse< char_type >( it, formula.end(), fmt ) ) {

                    if ( it == formula.end() )
                        break; // complete
                    
                    if ( *it == '+' ) {
                        fmt.first.emplace_back( adportable::chem::atom_type( 0, " +" ), 0 ); // put [+|-] in the text
                        ++it;
                    } else if ( *it == '-' ) {
                        fmt.first.emplace_back( adportable::chem::atom_type( 0, " -"), 0 ); // put [+|-] in the text
                        ++it;                    
                    }
                }
            }

            template< typename char_type > void print_text( std::basic_ostream< char_type >& o, const iformat_type&, bool );
        };
        template<> void formatter::print_text( std::basic_ostream< wchar_t >& o, const iformat_type& fmt, bool richText );
        template<> void formatter::print_text( std::basic_ostream< char >& o, const iformat_type& fmt, bool richText );

        /////////

        inline double monoIsotopicMass( const adportable::chem::icomp_type& comp, bool handleCharge = true )
        {
            adcontrols::TableOfElement *toe = adcontrols::TableOfElement::instance();
            double mass = 
                std::accumulate( comp.first.begin(), comp.first.end(), 0.0, [=]( double m, const adportable::chem::comp_type::value_type& pair ){
                        if ( mol::element e = toe->findElement( pair.first.second ) )
                            return m + mol::element::monoIsotopicMass( e, pair.first.first /* isotope */ ) * pair.second;
                        return m;
                    });

            if ( handleCharge ) {
                if ( comp.second > 0 ) {
                    return ( mass - ( toe->electronMass() * comp.second ) ) / comp.second;
                } else if ( comp.second < 0 ) {
                    return mass / std::abs( comp.second );
                }
            }

            return mass;
        }

        inline double chemicalMass( const adportable::chem::icomp_type& comp )
        {
            adcontrols::TableOfElement *toe = adcontrols::TableOfElement::instance();
            return 
                std::accumulate( comp.first.begin(), comp.first.end(), 0.0, [=]( double m, const adportable::chem::comp_type::value_type& pair ){
                        if ( mol::element e = toe->findElement( pair.first.second ) )
                            return m + mol::element::chemicalMass( e ) * pair.second;
                        return m;
                    });
        }

        template< typename char_type > std::basic_string<char_type>
        standardFormula( const std::basic_string<char_type>& formula ) {

            typedef typename std::basic_string< char_type >::const_iterator iterator_type;
            adportable::chem::chemical_formula_parser< iterator_type, adportable::chem::formulaComposition, adportable::chem::icomp_type > comp_parser;

            typename std::basic_string<char_type>::const_iterator it = formula.begin();

            adportable::chem::icomp_type comp;

            if ( boost::spirit::qi::parse( it, formula.end(), comp_parser, comp ) ) {

                std::vector< std::pair< adportable::chem::atom_type, size_t > > orderd;
                for ( auto atom: comp.first )
                    orderd.emplace_back( atom.first, atom.second );
                
                // reorder elements in alphabetical order
                std::sort( orderd.begin(), orderd.end()
                           , []( const std::pair< atom_type, size_t >& lhs, const std::pair< atom_type, size_t >& rhs ){
                               return std::strcmp( lhs.first.second, rhs.first.second ) < 0 || lhs.first.first < rhs.first.first;
                           } );
                
                std::basic_ostringstream<char_type> o;
                if ( comp.second )
                    o << "[";
                for ( auto& p: orderd ) {
                    std::basic_string<char_type> atom( p.first.second, p.first.second + std::strlen( p.first.second ) );
                    if ( p.first.first == 0 ) {
                        o << atom;
                        if ( p.second > 1 )  // omit '1' such as CH4, not C1H4
                            o << p.second;
                    } else {
                        o << atom;
                        if ( p.second > 1 ) 
                            o << p.second;
                        o << ' ';
                    }
                }
                if ( comp.second ) {
                    o << "]";
                    if ( std::abs( comp.second ) > 1 )
                        o << std::abs( comp.second );
                    o << ( comp.second < 0 ? '-' : '+' );
                }
                return o.str();
            }
            return std::basic_string<char_type>();
        };

        template<typename char_type> std::basic_string<char_type>
        make_adduct_string( const std::vector< std::pair < std::basic_string<char_type>, char_type > >& vec ) {
            std::basic_string<char_type> result;
            for ( auto formula: vec ) {
                if ( formula.second == char_type( '+' ) || formula.second == char_type( '-' ) ) {
                    result += formula.second;
                    result += formula.first;
                }
            }
            return result;
        }

        template<typename char_type> std::basic_string<char_type>
        make_formula_string( const std::vector< std::pair < std::basic_string<char_type>, char_type > >& vec ) {
            return
                std::accumulate( vec.begin(), vec.end(), std::basic_string<char_type>()
                                , []( const std::basic_string< char_type >& a, const std::pair < std::basic_string<char_type>, char_type >& b ) {
                                      return b.second == char_type('\0') ? ( a + b.first ) : a;  });
        }
        /////////////////
    } // namespace chem

} // namespace adcontrols


ChemicalFormula::~ChemicalFormula(void)
{
}

ChemicalFormula::ChemicalFormula() 
{
}

double
ChemicalFormula::getElectronMass() const
{
    return adcontrols::TableOfElement::instance()->electronMass();
}

double
ChemicalFormula::getMonoIsotopicMass( const std::wstring& formula ) const
{
    using namespace adportable::chem;
    chemical_formula_parser< std::wstring::const_iterator, formulaComposition, icomp_type > cf;

    std::wstring::const_iterator it = formula.begin();
    adportable::chem::icomp_type comp;    

    if ( boost::spirit::qi::parse( it, formula.end(), cf, comp ) ) {
        return chem::monoIsotopicMass( comp );
    }

    return 0;
}

double
ChemicalFormula::getMonoIsotopicMass( const std::string& formula ) const
{
    using namespace adportable::chem;
            
    chemical_formula_parser< std::string::const_iterator, formulaComposition, icomp_type > cf;

    std::string::const_iterator it = formula.begin();
    adportable::chem::icomp_type comp;

    if ( boost::spirit::qi::parse( it, formula.end(), cf, comp ) ) {
        return chem::monoIsotopicMass( comp );
    }

    return 0;
}

double
ChemicalFormula::getMonoIsotopicMass( const std::vector< std::pair< std::string, char > >& formulae ) const
{
    using namespace adportable::chem;
    chemical_formula_parser< std::string::const_iterator, formulaComposition, icomp_type > comp_parser;

    adportable::chem::icomp_type comp, lose;
    
    for ( auto& formula: formulae ) {
        if ( formula.second == '-' ) {
            std::string::const_iterator it = formula.first.begin();
            boost::spirit::qi::parse( it, formula.first.end(), comp_parser, lose );
        } else {
            std::string::const_iterator it = formula.first.begin();
            boost::spirit::qi::parse( it, formula.first.end(), comp_parser, comp );
        }
    }

    int charge = comp.second + lose.second;

    double mass = chem::monoIsotopicMass( comp, false );
    mass -= chem::monoIsotopicMass( lose, false );

    if ( charge > 0 ) {
        return ( mass - ( adcontrols::TableOfElement::instance()->electronMass() * charge ) ) / charge;
    } else if ( charge < 0 ) {
        return mass / std::abs(charge);
    }
    return mass;
}

double
ChemicalFormula::getChemicalMass( const std::wstring& formula ) const
{
    using namespace adportable::chem;
    chemical_formula_parser< std::wstring::const_iterator, formulaComposition, icomp_type > cf;

    std::wstring::const_iterator it = formula.begin();
    adportable::chem::icomp_type comp;    

    if ( boost::spirit::qi::parse( it, formula.end(), cf, comp ) )
        return chem::chemicalMass( comp );

    return 0;
}

std::wstring
ChemicalFormula::standardFormula( const std::wstring& formula )
{
    return chem::standardFormula<wchar_t>( formula );
}

std::string
ChemicalFormula::standardFormula( const std::string& formula )
{
    return chem::standardFormula<char>( formula );
}

std::string
ChemicalFormula::formatFormula( const std::string& formula, bool richText )
{
    chem::formatter formatter;

    chem::iformat_type fmt;
    std::string::const_iterator it = formula.begin();

    std::ostringstream o;
    if ( formatter.parse<char>( it, formula.end(), fmt ) )
        formatter.print_text( o, fmt, richText );

    assert( it == formula.end() );

    return o.str();
}

std::wstring
ChemicalFormula::formatFormula( const std::wstring& formula, bool richText )
{
    chem::formatter formatter;

    chem::iformat_type fmt;
    std::wstring::const_iterator it = formula.begin();

    std::wostringstream o;
    if ( formatter.parse<wchar_t>( it, formula.end(), fmt ) )
        formatter.print_text( o, fmt, richText );

    assert( it == formula.end() );

    return o.str();    
}


namespace adcontrols {

	//struct is_connect {
	size_t bond_connect( const CTable::Bond& bond, size_t num ) {
		return ( size_t( bond.first_atom_number ) == num ) ? bond.second_atom_number
			: ( size_t( bond.second_atom_number ) == num ) ? bond.first_atom_number : 0;
	}
}

bool
ChemicalFormula::getComposition( std::vector< mol::element >& el, const std::string& formula, int& charge )
{
    using namespace adportable::chem;
    chemical_formula_parser< std::string::const_iterator, formulaComposition, icomp_type > comp_perser;

    charge = 0;
    std::string::const_iterator it = formula.begin();
    adportable::chem::icomp_type comp;    

    if ( boost::spirit::qi::parse( it, formula.end(), comp_perser, comp ) ) {

        for ( auto& c: comp.first ) {
            // ignore isotope
            auto it = std::find_if( el.begin(), el.end(), [=]( const mol::element& e ){ return c.first.second == e.symbol(); }); 
            if ( it != el.end() ) {
                it->count( it->count() + c.second );
            } else {
                if ( mol::element e = TableOfElement::instance()->findElement( c.first.second ) ) {
                    e.count( c.second );
                    el.push_back( e );
                }
            }
        }
        charge = comp.second;
    }
}

//static
std::vector< std::pair<std::string, char > >
ChemicalFormula::split( const std::string& formula )
{
    using namespace adportable::chem;
    chemical_formula_parser< std::string::const_iterator, formulaComposition, icomp_type > comp_parser;

    std::vector< std::pair< std::string, char > > list;
    
    std::string::const_iterator it = formula.begin();
    adportable::chem::icomp_type comp;

    std::string::size_type pos = 0;
    char separator = 0;

    while ( boost::spirit::qi::parse( it, formula.end(), comp_parser, comp ) ) {

        auto count = std::distance( formula.begin(), it ) - pos;

        list.emplace_back( formula.substr( pos, count ), separator );

        if ( it == formula.end() )
            break;

        if ( *it == '+' || *it == '-' )
            separator = *it++;

        pos = std::distance( formula.begin(), it );
    }

    return list;
}

std::string
ChemicalFormula::formatFormulae( const std::string& formula, bool richText )
{
    chem::iformat_type fmt;
    chem::formatter().formulae<char>( fmt, formula );

    std::ostringstream o;
    chem::formatter().print_text<char>( o, fmt, richText );
    return o.str();
}

std::wstring
ChemicalFormula::formatFormulae( const std::wstring& formula, bool richText )
{
    chem::iformat_type fmt;
    chem::formatter().formulae<wchar_t>( fmt, formula );

    std::wostringstream o;
    chem::formatter().print_text<wchar_t>( o, fmt, richText );
    return o.str();    
}

std::string
ChemicalFormula::make_adduct_string( const std::vector< std::pair< std::string, char > >& list )
{
    return chem::make_adduct_string( list );
}

std::string
ChemicalFormula::make_formula_string( const std::vector< std::pair< std::string, char > >& list )
{
    return chem::make_formula_string( list );
}


//static
std::vector< std::pair<std::string, char > >
ChemicalFormula::splitAdducts( const std::string& adducts )
{
    typedef char char_type;
    typedef boost::tokenizer< boost::char_separator< char_type >
                              , typename std::basic_string< char_type >::const_iterator
                              , typename std::basic_string< char_type > > tokenizer_t;

    boost::char_separator< char_type > separator( ",; \t", "", boost::drop_empty_tokens );
    tokenizer_t tokens( adducts, separator );

    std::vector< std::pair<std::string, char> > list;

    for( auto it = tokens.begin(); it != tokens.end(); ++it ) {
        if ( it->length() > 0 ) {
            if ( ( *it )[ 0 ] == '+' || ( *it )[ 0 ] == '-' ) {
                char sign = ( *it )[ 0 ];
                std::string tok = *it;
                list.push_back( std::make_pair( tok.substr( 1 ), sign ) );
            } else {
                list.push_back( std::make_pair( *it, '+' ) );
            }
        }
    }
    return list;
}

std::string
ChemicalFormula::standardFormula( const std::vector< std::pair< std::string, char > >& formulae )
{
    std::string mformula, lformula;

    for ( auto& formula: formulae ) {
        if ( formula.second == '-' )
            lformula += formula.first;
        else
            mformula += formula.first;
    }

    int charge;
    std::vector< mol::element > mol, loses;
    getComposition( mol, mformula, charge );

    if ( getComposition( loses, lformula, charge ) ) {
        for ( auto& lose : loses ) {
            auto it = std::find_if( mol.begin(), mol.end(), [lose] ( const mol::element& a ) {
                    return a.atomicNumber() == lose.atomicNumber();
                } );
            if ( it != mol.end() )
                it->count( it->count() - lose.count() );
        }
    }
    std::sort( mol.begin(), mol.end(), [] ( const mol::element& a, const mol::element& b ) { return std::strcmp( a.symbol(), b.symbol() ) < 0; } );

    std::ostringstream o;
    for ( auto& a : mol ) {
        if ( a.count() > 0 ) {
            o << a.symbol();
            if ( a.count() > 1 )
                o << a.count();
        }
    }
    return o.str();
}

std::vector< std::string >
ChemicalFormula::standardFormulae( const std::string& formula, const std::string& adducts, std::vector< std::string >& adductlist )
{
    auto vec = splitAdducts( adducts );
    std::vector< std::string > formulae;
    for ( auto& adduct : vec ) {
        if ( adduct.second == '+' ) {
            auto sformula = standardFormula( formula + adduct.first );
            formulae.push_back( sformula );
            adductlist.push_back( "+" + adduct.first );            
        } else {
            int charge;
            std::vector< mol::element > mol, loses;
            getComposition( mol, formula, charge );
            if ( getComposition( loses, adduct.first, charge ) ) {
                for ( auto& lose : loses ) {
                    auto it = std::find_if( mol.begin(), mol.end(), [lose] ( const mol::element& a ) {
                                                 return a.atomicNumber() == lose.atomicNumber();
                                            } );
                    if ( it != mol.end() )
                        it->count( it->count() - lose.count() );
                }

                std::ostringstream o;
                for ( auto& a : mol ) {
                    if ( a.count() > 0 ) {
                        o << a.symbol();
                        if ( a.count() > 1 )
                            o << a.count();
                    }
                }
                formulae.push_back( o.str() );
                adductlist.push_back( "-" + adduct.first );
            }
        }
    }

    return formulae;
}

std::vector< std::string >
ChemicalFormula::standardFormulae( const std::string& formula, const std::string& adducts )
{
    std::vector< std::string > list;
    return standardFormulae( formula, adducts, list );
}


template<> void
chem::formatter::print_text( std::basic_ostream< wchar_t >& o, const iformat_type& fmt, bool richText )
{
    if ( fmt.second ) // has charge
        o << L"[";
    for ( auto e: fmt.first ) {
        if ( std::strcmp( e.first.second, "(" ) == 0 ) { // open repeat-group
            o << L"(";
        } else if ( std::strcmp( e.first.second, ")" ) == 0 ) { // close repeat-group
            o << boost::wformat( L")<sub>%1%</sub>" ) % e.second;
        } else {
            if ( e.first.first )
                o << boost::wformat( L"<sup>%1%</sup>" ) % e.first.first; // element's atomic weight

            o << boost::wformat( L"%s" ) % e.first.second; // element name

            if ( e.second > 1 )
                o << boost::wformat( L"<sub>%1%</sub>" ) % e.second;
        }
    }
    if ( fmt.second ) {
        if ( fmt.second > 1 )
            o << "]<sup>" << std::abs(fmt.second) << (fmt.second < 0 ? '-' : '+') << "</sup>";
        else
            o << "]<sup>" << (fmt.second < 0 ? '-' : '+') << "</sup>";
    }
}

template<> void
chem::formatter::print_text( std::basic_ostream< char >& o, const iformat_type& fmt, bool richText )
{
    if ( fmt.second ) // has charge
        o << "[";
    for ( auto e: fmt.first ) {
        if ( std::strcmp( e.first.second, "(" ) == 0 ) { // open repeat-group
            o << "(";
        } else if ( std::strcmp( e.first.second, ")" ) == 0 ) { // close repeat-group
            o << boost::format( ")<sub>%1%</sub>" ) % e.second;
        } else {
            if ( e.first.first )
                o << boost::format( "<sup>%1%</sup>" ) % e.first.first; // element's atomic weight

            o << boost::format( "%s" ) % e.first.second; // element name

            if ( e.second > 1 )
                o << boost::format( "<sub>%1%</sub>" ) % e.second;
        }
    }
    
    if ( fmt.second ) {
        if ( fmt.second > 1 )
            o << "]<sup>" << std::abs(fmt.second) << (fmt.second < 0 ? '-' : '+') << "</sup>";
        else
            o << "]<sup>" << (fmt.second < 0 ? '-' : '+') << "</sup>";
    }
}
