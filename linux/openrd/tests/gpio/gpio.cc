
#include <stdio.h>
#include <cstdio>
#include <iostream>
#include <sstream>

main()
{
    FILE * fp = fopen( "/sys/class/gpio/export", "ab" );
    if ( ! fp ) {
        std::cerr << "/sys/class/gpio/export open failed" << std::endl;
        return 0;
    }

    for ( int i = 0; i < 50; ++i ) {
        rewind( fp );
        std::ostringstream o;
        o << i;
        size_t r = fwrite( o.str().c_str(), sizeof(char), o.str().size(), fp );
        std::cerr << "fwrite: " << r << std::endl;
    }
    fclose(fp);
}
