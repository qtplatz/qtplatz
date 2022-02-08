// https://stackoverflow.com/questions/50821925/spirit-x3-parser-with-internal-state
#define BOOST_SPIRIT_X3_DEBUG
#include <iostream>
#include <iomanip>
#include <boost/spirit/home/x3.hpp>
#include <boost/variant.hpp>

namespace x3 = boost::spirit::x3;

namespace CSV {
    struct text    {};
    struct integer {};
    struct real    {};
    struct skip    {};

    struct handler {
        template< typename T > void operator()( T& ctx ) {
            std::cout << _attr(ctx) << std::endl;
        }
    };

    auto const unquoted_text_field = *~x3::char_(",\n");
    static inline auto as_parser(skip)    { return x3::omit[unquoted_text_field];  }
    static inline auto as_parser(text)    { return unquoted_text_field[handler()]; }
    static inline auto as_parser(integer) { return x3::int_[handler()];            }
    static inline auto as_parser(real)    { return x3::double_[handler()];         }

    template <typename... Spec>
    static inline auto line_parser(Spec... spec) {
        auto delim = x3::char_(",\t ") >> *x3::char_("\t ") | &(x3::eoi | x3::eol); // this ignores continuous comma ,,,
#ifdef BOOST_SPIRIT_X3_DEBUG
        auto debugged = [](auto spec) {
            using boost::core::demangle_alloc; // leaked on purpose
            return x3::rule<struct _> {demangle_alloc(typeid(spec).name())} = as_parser(spec);
        };
        return x3::rule<struct __> {"line"} = ((debugged(spec) >> delim) >> ... >> x3::eps);
#else
        return ((as_parser(spec) >> delim) >> ... >> x3::eps);
#endif
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
    std::string const input = "Hello\t,\t1,13.7,XXX\nWorld,2,1e3,YYY";
    auto f = begin(input), l = end(input);

    //auto p = csv_parser(text{}, integer{}, real{}, skip{});
    auto p = csv_parser( text{}, integer{}, real{}, skip{} );

    if (parse(f, l, p)) {
        std::cout << "Parsed\n";
    } else {
        std::cout << "Failed\n";
    }

    if (f!=l) {
        std::cout << "Remaining: " << std::quoted(std::string(f,l)) << "\n";
    }
}
