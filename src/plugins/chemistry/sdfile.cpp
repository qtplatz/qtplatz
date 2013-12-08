/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "sdfile.hpp"
#include <adportable/debug.hpp>

#include <RDGeneral/Invariant.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Substruct/SubstructMatch.h>
#include <GraphMol/Depictor/RDDepictor.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <RDGeneral/RDLog.h>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/map.hpp>

namespace chemistry {
    namespace client {

        struct structure_data_t {
            std::string mol;
            std::map< std::string, std::string > data;
        };

        typedef std::pair<std::string, std::string > pair_type;
        
        struct handler_t {
            static void assign( pair_type& t, const std::string& d ) { 
            }
            static void assign( structure_data_t& t, const std::string& d ) { 
                t.mol = d;
            }
            static void assign( structure_data_t& t, const std::pair<std::string, std::string>& d ) {
                t.data[ d.first ] = d.second;
            }
        };

        
        namespace qi = boost::spirit::qi;
        using qi::lit;
        using qi::lexeme;
        using qi::_val;
        using qi::_1;
        using boost::spirit::ascii::char_;
        using boost::spirit::ascii::space;
        using boost::phoenix::bind;
        using boost::phoenix::at_c;

        template<typename Iterator>
        struct sdfile_parser : boost::spirit::qi::grammar< Iterator, pair_type() > {
            
            sdfile_parser() : sdfile_parser::base_type( node )  {
                
                text = lexeme[+(char_ - '<') [_val += _1]];

                start_tag = '>' >> lit('<')
                                >> lexeme[+(char_ - '>') [_val += _1]]
                                >> '>'
                    ;

                node = 
                    start_tag  [ boost::phoenix::bind(handler_t::assign, _val, _1) ]
                    >> text    [ boost::phoenix::bind(handler_t::assign, _val, _1) ]
                    ;
            }

            qi::rule<Iterator, std::string(), boost::spirit::ascii::space_type > token;
            qi::rule<Iterator, std::string(), boost::spirit::ascii::space_type > text;
            qi::rule<Iterator, std::string(), boost::spirit::ascii::space_type > start_tag;
            qi::rule<Iterator, std::string(), boost::spirit::ascii::space_type > quoted_string;
            qi::rule<Iterator, pair_type() > node;
        };

    }
}

using namespace chemistry;

SDFile::SDFile()
{
}

SDFile::SDFile( const std::string& filename, bool sanitize, bool removeHs, bool strictParsing )
    : molSupplier_( std::make_shared< RDKit::SDMolSupplier >( filename, sanitize, removeHs, strictParsing ) )
    , filename_( filename )
{
    adportable::debug(__FILE__, __LINE__) << filename_;
    adportable::debug(__FILE__, __LINE__) << molSupplier_->length();
}

// static
bool
SDFile::associatedData( const std::string& text, std::map< std::string, std::string >& data )
{
    data.clear();

    client::sdfile_parser< std::string::const_iterator > parser;

    std::string::const_iterator it = text.begin();
    std::string::const_iterator end = text.end();

    client::pair_type pair;

    boost::spirit::qi::parse( it, end, parser, pair );

    (void)text;
}

#if 0

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

#endif
