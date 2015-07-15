
#include <iostream>
#include <adportable/split_filename.hpp>

using namespace adportable;

int
main()
{
    for ( auto& a: { "RUN_0001", "123459989", "123456x89", "1", "12", "123", "abc", "" } ) {

        std::cout << "prefix:trail\t"
                  << split_filename::prefix<char>( a )
                  << ":" << split_filename::trailer_number<char>( a )
                  << "\tas int:" << split_filename::trailer_number_int<char>( a )
                  << std::endl;

    }
}
