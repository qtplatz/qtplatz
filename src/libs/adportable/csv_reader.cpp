// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC
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

#include "csv_reader.hpp"
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/variant.hpp>
#include <variant>
#include <fstream>

using namespace adportable;
using namespace adportable::csv;

namespace adportable {

    class csv_reader::impl {
    public:
        std::ifstream istrm_;
        impl( const std::string& file ) : istrm_( file ) {
        }
    };

    namespace csv {

        namespace x3 = boost::spirit::x3;

        /////////// input line to string array parser
        auto const quoted_string = x3::lexeme['"' >> +(x3::char_ - '"') >> '"'];
        auto const unquoted_string = *~x3::char_(",\n\t");
        static inline auto csv_parser() {
            auto delim = ( x3::char_(",\t ") >> *(x3::char_("\t ")) );
            return
                ( quoted_string
                  | unquoted_string
                    )
                % delim
                >> (x3::eoi | x3::eol)
                ;
        }
    } // csv

    struct integer {};
    struct real    {};
    struct null    {};
    static inline auto as_parser(integer) { return x3::int_    ;  }
    static inline auto as_parser(real)    { return x3::double_ ;  }

    template < typename... specs > struct type_parser_list {};

    template < typename last_t > struct type_parser_list< last_t > {
        template< typename value_t >
        bool operator()( const std::string& s, value_t& ) const {
            return {}; //false;
        }
    };

    template< typename first_t, typename... specs > struct type_parser_list< first_t, specs ... > {
        template< typename value_t >
        bool operator()( const std::string& s, value_t& v ) const {
            auto f = std::begin( s ), l = std::end( s );
            if ( x3::parse( f, l, as_parser( first_t{} ), v ) && ( f == l ) ) {
                return true;
            }
            return type_parser_list< specs ... >()( s, v );
        }
    };

}

csv_reader::csv_reader( const std::string& file )
    : impl_( std::make_unique< impl >( file ) )
{
}

bool
csv_reader::read( list_type& list )
{
    list = {};
    std::string line;
    if ( std::getline( impl_->istrm_, line ) ) {
        namespace x3 = boost::spirit::x3;
        auto first( std::begin( line ) ), last( std::end( line ) );

        if ( x3::parse( first, last, csv::csv_parser(), list ) ) {
            if ( first == last ) {
                for ( auto& value: list ) {
                    if ( value.type() == typeid( std::string() ) ) {
                        const auto& a = boost::get< std::string >( value );
                        csv::variant_type t;
                        if ( type_parser_list< integer, real, null >()( a, t ) )
                            value = std::move( t );
                    }
                }
                return true;
            }
        }
    }
    return false;
}
