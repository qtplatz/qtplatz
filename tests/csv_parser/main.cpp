// https://stackoverflow.com/questions/50821925/spirit-x3-parser-with-internal-state
//#define BOOST_SPIRIT_X3_DEBUG
#include <iostream>
#include <iomanip>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/variant.hpp>

namespace x3 = boost::spirit::x3;

typedef boost::variant< std::string, int, double, boost::spirit::x3::unused_type > variant_type;

namespace CSV {
    struct text    {};
    struct integer {};
    struct real    {};
    struct skip    {};
    struct number  {};

    struct visitor : boost::static_visitor< void > {
        template< typename T > void operator()( T&& t ) const {
            std::cout << "\t" << typeid( t ).name() << ":\t" << t << std::endl;
        }
        void operator()( double&& t ) const {
            std::cout << "\tdouble:\t" << t << std::endl;
        }
        void operator()( int&& t ) const {
            std::cout << "\tint:\t" << t << std::endl;
        }
        void operator()( std::string&& t ) const {
            std::cout << "\tstring:\t'" << t << "'" << std::endl;
        }
    };

    struct handler {
        template< typename T > void operator()( T& ctx ) const {
            auto value = _attr(ctx);
            // std::cout << typeid( value ).name() << "\t" << value << std::endl;
            boost::apply_visitor( visitor(), variant_type( value ) );
        }
    };

    auto const unquoted_text_field = *~x3::char_(",\n");
    static inline auto as_parser(skip)    { return x3::omit[unquoted_text_field];  }
    static inline auto as_parser(text)    { return ( unquoted_text_field | x3::attr(0) ); }
    static inline auto as_parser(integer) { return ( x3::int_    | x3::attr(0) );  }
    static inline auto as_parser(real)    { return ( x3::double_ | x3::attr(0) );  }
    static inline auto as_parser(number)  { return ( x3::double_ | x3::int_ | x3::attr(0) ); }

    template <typename... Spec>
    static inline auto line_parser(Spec... spec) {
        auto delim = x3::char_(",\t ") >> *x3::char_("\t ") | &(x3::eoi | x3::eol); // this ignores continuous comma ,,,
        return ((as_parser(spec) [ handler() ] >> delim) >> ... >> x3::eps);
    }

    template <typename... Spec> static inline auto csv_parser(Spec... spec) {
        return line_parser(spec...) % x3::eol;
    }
}

using namespace CSV;

int
main()
{
    //std::string const input = "Hello,1,13.7,XXX\nWorld,2,1e3,YYY";
    std::string const input = "Hello\t,\t1,13.7,\"XXX\"\n\"World\",2,1e3,YYY\n,,,YYY";
    auto f = begin(input), l = end(input);

    //auto p = csv_parser(text{}, integer{}, real{}, skip{});
    //auto p = csv_parser( text{}, integer{}, real{}, skip{} );
    auto p = csv_parser( text{}, number{}, number{}, skip{} );

    if (parse(f, l, p)) {
        std::cout << "Parsed\n";
    } else {
        std::cout << "Failed\n";
    }

    if (f!=l) {
        std::cout << "Remaining: " << std::quoted(std::string(f,l)) << "\n";
    }
}
