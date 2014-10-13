
#pragma once

namespace test {
    
    namespace xml {

        class archive {
        public:
            static const char * title() { return "XML\tSerializer"; }
            static const char * file_extension() { return ".xml"; }

            template< class T > bool save( const T& t, const char * file ) {
                try {

                    std::wofstream fo( file );
                    return adportable::xml::serialize<>()(t, fo);

                } catch ( ... ) {
                    std::cout << "\t" << boost::current_exception_diagnostic_information();
                    return false;
                }
            }

            template< class T > bool load( T& t, const char * file ) {
                try {
                    std::wifstream fi( file );
                    return adportable::xml::deserialize<>()(t, fi); // T::xml_restore( fi, t );
                } catch ( ... ) {
                    std::cout << "\t" << boost::current_exception_diagnostic_information();
                    return false;
                }
            }
        };
    }
}
