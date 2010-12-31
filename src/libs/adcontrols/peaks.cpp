//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "peaks.h"
#include "peak.h"
#include "baseline.h"
#include "baselines.h"

using namespace adcontrols;

Peaks::~Peaks()
{
}

Peaks::Peaks()
{
}

Peaks::Peaks( const Peaks& t ) : peaks_( t.peaks_ )
{
}

void
Peaks::add( const Peak& pk )
{
    peaks_.push_back( pk );
}

///////////////

Peaks::vector_type::const_iterator
Peaks::find_first_peak( const Baseline& bs ) const
{
    for ( vector_type::const_iterator it = begin(); it != end(); ++it ) {
        if ( bs.startTime() <= it->startTime() && it->endTime() <= bs.stopTime() )
            return it;
    }
    return end();
}

Peaks::vector_type::iterator
Peaks::find_first_peak( const Baseline& bs )
{
    for ( vector_type::iterator it = begin(); it != end(); ++it ) {
        if ( bs.startTime() <= it->startTime() && it->endTime() <= bs.stopTime() )
            return it;
    }
    return end();
}