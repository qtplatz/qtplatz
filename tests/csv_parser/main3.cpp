// https://stackoverflow.com/questions/50821925/spirit-x3-parser-with-internal-state
//#define BOOST_SPIRIT_X3_DEBUG

#include <adportable/csv_reader.cpp>
#include <boost/format.hpp>
#include <iostream>
#include <iomanip>
#include <type_traits>

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

template<class Ch, class Tr, class Tuple, std::size_t... Is>
void print_tuple_impl(std::basic_ostream<Ch,Tr>& os, const Tuple& t, std::index_sequence<Is...>)
{
    ((os << (Is == 0? "" : ", ") << std::get<Is>(t)), ...);
}

template<class Ch, class Tr, class... Args>
auto& operator<<(std::basic_ostream<Ch, Tr>& os, const std::tuple<Args...>& t)
{
    os << "(";
    print_tuple_impl(os, t, std::index_sequence_for<Args...>{});
    return os << ")";
}

namespace csv {

    template< typename T >
    struct value_to : boost::static_visitor < T > {
        template<typename V> T operator()( V& v ) const {
            return v;
        }

        T operator()( const boost::spirit::x3::unused_type& v ) const {
            return {};
        }

        T operator()( const std::string& v ) const {
            return {};
        }
    };

    template< typename Tuple, std::size_t... Is>
    Tuple to_tuple_impl( const list_type& list, Tuple& t, std::index_sequence<Is...> )
    {
        (( std::get<Is>(t) = boost::apply_visitor( value_to< std::tuple_element_t<Is, Tuple> >(), list[Is] ) ), ...); // <-- this works
        return t;
    }

    template<class... Args>
    std::tuple<Args...>
    to_tuple( const list_type& list )
    {
        std::tuple<Args...> t;
        return make_tuple_impl( list, t, std::index_sequence_for<Args...>{});
    }
}


int
main( int argc, const char * const argv[] )
{
    if ( argc > 1 ) {
        adportable::csv::csv_reader reader( argv[ 1 ] );
        adportable::csv::list_type list;
        size_t row(0);
        while ( reader.read( list ) ) {
            for ( const auto& value: list ) {
                boost::apply_visitor( print_visitor(), value );
            }
            ++row;
            std::cout << std::endl;
        }
    } else {
        adportable::csv::csv_reader reader;
        adportable::csv::list_type list;
        size_t row(0);

        typedef std::tuple< double, double, double, int > data_type;

        while ( reader.read( std::cin, list ) ) {
            auto data = adportable::csv::to_tuple< double, double, double, int >( list );
            std::cout << data << std::endl;
            ++row;
        }
    }
}
