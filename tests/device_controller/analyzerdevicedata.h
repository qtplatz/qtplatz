#pragma once

#include "tofcontrollerC.h"

namespace tofcontroller {

    template<class T> class copy_helper {
    public:
        static void copy( T& d, const T& s ) {
            TAO_OutputCDR out;
            out << s;
            TAO_InputCDR in( out );
            in >> d;
        }
    };

}
