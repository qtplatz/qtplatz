// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "timeutil.h"

using namespace adcontrols;

seconds_t::seconds_t( const seconds_t& t ) : seconds( t.seconds )
{
}

seconds_t::seconds_t( const minutes_t& t ) : seconds( timeutil::toSeconds( t ) )
{
}

minutes_t::minutes_t( const minutes_t& t ) : minutes( t.minutes )
{
}

minutes_t::minutes_t( const seconds_t& t ) : minutes( timeutil::toMinutes( t ).minutes )
{
}

seconds_t
timeutil::toSeconds( const minutes_t& m )
{
    return m.minutes * 60.0;
}

minutes_t
timeutil::toMinutes( const seconds_t& s )
{
    return s.seconds / 60.0;
}

std::pair<double, double>
timeutil::toMinutes( const std::pair<seconds_t, seconds_t>& pair )
{
    return std::make_pair( pair.first.seconds / 60.0, pair.second.seconds / 60.0 );
}
