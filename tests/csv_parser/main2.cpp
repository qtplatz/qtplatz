#include <iostream>
#include <variant>

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/support.hpp>

namespace helpers {
    // https://bitbashing.io/std-visit.html
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
}

auto const unquoted_text_field = *(boost::spirit::x3::char_ - ',' - boost::spirit::x3::eol);

struct text { };
struct integer { };
struct real { };
struct skip { };
typedef std::variant<text, integer, real, skip> column_variant;

struct column_value_parser : boost::spirit::x3::parser<column_value_parser> {
    typedef boost::spirit::unused_type attribute_type;

    std::vector<column_variant>& columns;

    column_value_parser(std::vector<column_variant>& columns)
        : columns(columns)
    { }

    template<typename It, typename Ctx, typename Other, typename Attr>
    bool parse(It& f, It l, Ctx const& ctx, Other const& /*other*/, Attr& /*attr*/) const {
        auto const saved_f = f;
        bool successful = false;

        size_t& pos = boost::spirit::x3::get<pos_tag>(ctx).value;

        visit(
            helpers::overloaded {
                [&](skip const&) {
                    successful = boost::spirit::x3::parse(f, l, boost::spirit::x3::omit[unquoted_text_field]);
                },
                [&](text&) {
                    std::string value;
                    successful = boost::spirit::x3::parse(f, l, unquoted_text_field, value);
                    if(successful) {
                        std::cout << "Text: " << value << '\n';
                    }
                },
                [&](integer&) {
                    int value;
                    successful = boost::spirit::x3::parse(f, l, boost::spirit::x3::int_, value);
                    if(successful) {
                        std::cout << "Integer: " << value << '\n';
                    }
                },
                [&](real&) {
                    double value;
                    successful = boost::spirit::x3::parse(f, l, boost::spirit::x3::double_, value);
                    if(successful) {
                        std::cout << "Real: " << value << '\n';
                    }
                }
            },
            columns[pos]);

        if(successful) {
            pos = (pos + 1) % columns.size();
            return true;
        } else {
            f = saved_f;
            return false;
        }
    }

    template <typename T>
    struct Mutable { T mutable value; };
    struct pos_tag;

    auto invoke() const {
        return boost::spirit::x3::with<pos_tag>(Mutable<size_t>{}) [ *this ];
    }
};


int main() {
    std::string input = "Hello,1,13.7,XXX\nWorld,2,1e3,YYY";

    std::vector<column_variant> columns = {text{}, integer{}, real{}, skip{}};
    column_value_parser p(columns);

    boost::spirit::x3::parse(
        input.begin(), input.end(),
        (p.invoke() % ',') % boost::spirit::x3::eol);
}
