// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <cmath>
#include <float.h>

namespace adportable {

    namespace internal {
        template<class T> struct epsiron { static T value() { return 0; } 
        };

        template<> struct epsiron<double> { static double value() { return DBL_EPSILON; }
        };
        template<> struct epsiron<float> { static float value() { return FLT_EPSILON; }
        };
    }

    template<class T> bool is_equal(const T& a, const T& b ) {
        return std::abs( a - b ) < internal::epsiron<T>::value();
    }

}
