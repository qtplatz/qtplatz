
#include <iostream>
#include <boost/json.hpp>

int
main()
{
    std::cout << "Hello world" << std::endl;
    int i = 1;
    double d = 9.999;
    std::pair< int, int > pair{ 2, 3 };
    std::vector< int > v{ 4, 5 };

    boost::json::value jv = {{ "boost_i", i }
                             , { "boost_v", boost::json::value_from(v) }
                             , { "boost_pair", pair }
    };
    std::cout << jv << std::endl;

}
