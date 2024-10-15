// -*- C++ -*-
/**************************************************************************
**
** MIT License
** Copyright (c) 2021-2024 Toshinobu Hondo, Ph.D

** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:

** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**************************************************************************/

// #if not defined WIN32 // MSVC++ 14 can not compile spirit::x3

#include "csv_reader.hpp"
#include "debug.hpp"
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/variant.hpp>
#include <fstream>

using namespace adportable;
using namespace adportable::csv;

namespace adportable {

    class csv_reader::impl {
    public:
        std::ifstream istrm_;
        impl( const std::string& file ) : istrm_( file ) {
        }
        impl( std::ifstream&& inf ) : istrm_( std::move( inf ) ) {
        }
    };

    namespace csv {

        namespace x3 = boost::spirit::x3;

        /////////// input line to string array parser
        auto const quoted_string = x3::lexeme['"' >> +(x3::char_ - '"') >> '"'];
        auto const unquoted_string = *~x3::char_(",\r\n\t");

        // following code does not conforming to RFC 4180, where it does not allows to ignore '\t' following to separator
        static inline auto csv_parser() {
            auto delim = ( x3::char_(",\t") >> *(x3::char_("\t ")) ); // ignore white-spaces follows to delimiter (,\t )
            return
                ( quoted_string
                  | unquoted_string
                    )
                % delim
                >> (x3::eoi | x3::eol)
                ;
        }

        /////////////////
        // std::string
        template<> template<typename V> std::string to_value<std::string>::operator()(const V& v) const { return std::to_string( v ); }

        // int
        template<> template<> int to_value<int>::operator()(const std::string& v) const { try { return stoi(v); } catch ( ... ){}; return {}; }

        // double
        template<> template<> double to_value<double>::operator()(const std::string& v) const { try { return stod(v); } catch ( ... ){}; return {}; }

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

csv_reader::~csv_reader()
{
}

csv_reader::csv_reader()
{
}

csv_reader::csv_reader( const std::string& file )
    : impl_( std::make_unique< impl >( file ) )
{
}

csv_reader::csv_reader( std::ifstream&& ins )
    : impl_( std::make_unique< impl >( std::move( ins ) ) )
{
}

void
csv_reader::rewind()
{
    impl_ && impl_->istrm_.seekg( 0 );
}

bool
csv_reader::read( list_type& list )
{
    return impl_ && read( impl_->istrm_, list );
}

bool
csv_reader::read( std::istream& istrm, list_type& list )
{
    list = {};
// #if not defined _MSC_VER
    std::string line;
    if ( std::getline( istrm, line ) ) {
        namespace x3 = boost::spirit::x3;
        auto first( std::begin( line ) ), last( std::end( line ) );
        if ( x3::parse( first, last, csv::csv_parser(), list ) ) {
            if ( first == last ) {
                for ( auto& value: list ) {
                    if ( value.type() == typeid( std::string ) ) {
                        const auto& a = boost::get< std::string >( value );
                        variant_type t;
                        if ( type_parser_list< real, integer, null >()( a, t ) ) {
                            value = std::move( t );
                        }
                    }
                }
                return true;
            }
        }
    }
// #endif
    return false;
}

bool
csv_reader::read( std::istream& istrm, list_string_type& alist )
{
    alist = {};
    list_type list{};

    std::string line;
    if ( std::getline( istrm, line ) ) {

        namespace x3 = boost::spirit::x3;
        auto first( std::begin( line ) ), last( std::end( line ) );
        if ( x3::parse( first, last, csv::csv_parser(), list ) ) {
            if ( first == last ) {
                for ( const auto& value: list ) {
                    if ( value.type() == typeid( std::string ) ) {
                        const auto& a = boost::get< std::string >( value );
                        variant_type v;
                        if ( type_parser_list< real, integer, null >()( a, v ) ) {
                            alist.emplace_back( v, a );
                        } else {
                            alist.emplace_back( a, a );
                            ADDEBUG() << "\t======= csv_reader::read-else : " << a;
                        }
                    } else {
                        ADDEBUG() << "\t>>>>> csv_reader::read non-string value <<<<<<<<<<";
                    }
                }
                return true;
            }
        }
    }

    return false;
}

bool
csv_reader::skip( size_t nlines )
{
    return impl_ && skip( impl_->istrm_, nlines );
}

bool
csv_reader::skip( std::istream& istrm, size_t nlines )
{
    std::string line;
    while ( nlines ) {
        if ( ! std::getline( istrm, line ) )
            return false;
        --nlines;
    }
    return true;
}

// #endif
