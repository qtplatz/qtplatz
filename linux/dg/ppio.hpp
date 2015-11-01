
#include <cstdint>

class ppio {
    bool success_;
    static constexpr uint16_t BASEPORT = 0x378;
    unsigned char data_;
public:
    ppio();
    ~ppio();
    operator bool () const;
    ppio& operator << ( const unsigned char d );
};

