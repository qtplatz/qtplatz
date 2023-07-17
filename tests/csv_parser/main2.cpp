// https://stackoverflow.com/questions/50821925/spirit-x3-parser-with-internal-state
//#define BOOST_SPIRIT_X3_DEBUG
#include <iostream>
#include <iomanip>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/variant.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <fstream>

namespace x3 = boost::spirit::x3;

typedef boost::variant< boost::spirit::x3::unused_type, std::string, double, int > variant_type;
typedef std::vector< variant_type > list_type;

struct print_visitor : boost::static_visitor< void > {
    template< typename T >
    void operator()( T&& t ) const {
    }

    void operator()( const std::string& t ) const {
        std::cout << t;
    }
    void operator()( int t ) const {
        std::cout << boost::format( "%8d\t" ) % t;
    }
    void operator()( double t ) const {
        if ( std::abs(t) < 0.001 || std::abs(t) > 10000 )
            std::cout << boost::format( "%.5e\t" ) % t;
        else
            std::cout << boost::format( "%8.3f\t" ) % t;
    }
};


namespace CSV {

    struct integer {};
    struct real    {};
    struct null    {};
    static inline auto as_parser(integer) { return x3::int_    ;  }
    static inline auto as_parser(real)    { return x3::double_ ;  }

    template < typename... specs > struct type_parser_list {};

    template < typename last_t > struct type_parser_list< last_t > {
        template< typename value_t >
        bool operator()( const std::string& s, value_t& v ) {
            return false;
        }
    };

    template< typename first_t, typename... specs > struct type_parser_list< first_t, specs ... > {
        template< typename value_t >
        bool operator()( const std::string& s, value_t& v ) const {
            auto f = std::begin( s ), l = std::end( s );
            if ( x3::parse( f, l, as_parser( first_t{} ), v ) && f == l ) {
                return true;
            }
            return type_parser_list< specs ... >()( s, v );
        }
    };

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
}

using namespace CSV;

int
main( int argc, const char * const argv[] )
{
    if ( argc < 2 ) {
        //std::string const input = "Hello,1,13.7,XXX\nWorld,2,1e3,YYY";
        std::string const input = "Hello\tabc,,,\t999,13.7,,888abc,\t\"Hello\tWorld!\""; // \"World\",2,1e3,YYY\n,,,YYY";
        auto first = begin(input), last = end(input);

        auto p = csv_parser();
        list_type list;
        if ( x3::parse( first, last, p, list ) ) {
            std::cout << "Parsed\n";

            size_t n(0);
            for ( auto& value: list ) {
                std::cout << "[" << n++ << "] " << value.which() << ", '" << value << "'" << std::endl;
                if ( value.type() == typeid( std::string ) ) {
                    auto a = boost::get< std::string >( value );
                    if ( type_parser_list< integer, real, null >()( a, value ) )
                        std::cout << "\treplace type: " << value.type().name() << "\t" << value << std::endl;
                }
            }
        } else {
            std::cout << "Failed\n";
        }
        if ( first != last) {
            std::cout << "Remaining: " << std::quoted(std::string(first,last)) << "\n";
        }
    } else {
        auto file = boost::filesystem::path( argv[ 1 ] );
        if ( boost::filesystem::exists( file ) ) {
            std::ifstream inf( file.string() );
            std::string line;
            while ( std::getline( inf, line ) ) {
                auto first( std::begin( line ) ), last( std::end( line ) );
                auto p = csv_parser();
                list_type list;
                if ( x3::parse( first, last, p, list ) ) {
                    for ( auto& value: list ) {
                        if ( value.type() == typeid( std::string ) ) {
                            auto a = boost::get< std::string >( value );
                            type_parser_list< integer, real, null >()( a, value );
                            boost::apply_visitor( print_visitor(), variant_type( value ) );
                        }
                    }
                    std::cout << std::endl;
                } else {
                    std::cout << "\tFailed" << std::endl;
                }
                if ( first != last) {
                    std::cout << "\tRemaining: " << std::quoted(std::string(first,last)) << std::endl;
                }
            }
        }
    }
}
