//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "baselines.h"
#include "baseline.h"

using namespace adcontrols;

Baselines::~Baselines()
{
}

Baselines::Baselines() : nextId_(0)
{
}

Baselines::Baselines( const Baselines& t ) : nextId_( t.nextId_ )
                                           , baselines_( t.baselines_ ) 
{
}

long
Baselines::add( const Baseline& t )
{
    baselines_.push_back( t );
    baselines_.back().baseId( nextId_++ );
    return baselines_.back().baseId();
}
