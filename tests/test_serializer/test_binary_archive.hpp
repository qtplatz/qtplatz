

#pragma once

#include <fstream>

namespace test {

    namespace binary {

        class archive {
        public:
            static const char * title() { return "BINARY\t Serializer"; }

            static const char * file_extension() { return ".bin"; }
        
            template< class T > bool save( const T& t, const char * file ) {
                try {
                    std::ofstream fo( file );
                    return adportable::binary::serialize<>()(t, fo);
                } catch ( std::exception& ex ) {
                    std::cout << "\t" << boost::diagnostic_information( ex );
                    BOOST_THROW_EXCEPTION(ex);
                }
                return false;
            }
        
            template< class T > bool load( T& t, const char * file ) {
                try {
                    std::ifstream fi( file );
                    return adportable::binary::deserialize<>()(t, fi);
                } catch ( std::exception& ex ) {
                    std::cout << "\t" << boost::diagnostic_information( ex );
                    BOOST_THROW_EXCEPTION(ex);
                }
                return false;
            }
        };
    }
}
