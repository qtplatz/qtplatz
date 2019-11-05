// Copyright (C) 2018 MS-Cheminformatics LLC
// Licence: CC BY-NC
// Author: Toshinobu Hondo, Ph.D.
// Contact: toshi.hondo@qtplatz.com
//

#pragma once

#include <cstddef>

struct condition_wait {
    size_t count;
    condition_wait( size_t maxcounts = 0xffff ) : count( maxcounts ) {}
    template< typename functor >  inline bool operator()( functor condition ) {
        while ( --count && !condition() )
            ;
        return count != 0;
    }
};
