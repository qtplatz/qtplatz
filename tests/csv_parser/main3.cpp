// https://stackoverflow.com/questions/50821925/spirit-x3-parser-with-internal-state
//#define BOOST_SPIRIT_X3_DEBUG
#include <adportable/csv_reader.cpp>
#include <boost/format.hpp>
#include <iostream>
#include <iomanip>

namespace x3 = boost::spirit::x3;

struct print_visitor : boost::static_visitor< void > {
    template< typename T >
    void operator()( T&& t ) const {
        std::cout << typeid(t).name() << "\t";
    }

    void operator()( const std::string& t ) const {
        std::cout << boost::format( "s %s\t") % t;
    }
    void operator()( int t ) const {
        std::cout << boost::format( "i %8d\t" ) % t;
    }
    void operator()( double t ) const {
        if ( std::abs(t) < 0.001 || std::abs(t) > 10000 )
            std::cout << boost::format( "e %.5e;\t" ) % t;
        else
            std::cout << boost::format( "d %8.3f;\t" ) % t;
    }
};

int
main( int argc, const char * const argv[] )
{
    if ( argc > 1 ) {
        csv::csv_reader reader( argv[ 1 ] );
        csv::list_type list;
        size_t row(0);
        while ( reader.read( list ) ) {
            for ( const auto& value: list ) {
                boost::apply_visitor( print_visitor(), value );
            }
            ++row;
            std::cout << std::endl;
        }
    }
}
