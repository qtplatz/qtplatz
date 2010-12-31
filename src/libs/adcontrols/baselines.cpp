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

int
Baselines::add( const Baseline& t )
{
    baselines_.push_back( t );
    return baselines_.back().baseId();
}

int
Baselines::nextId( bool increment )
{
    if ( increment )
        ++nextId_;
    return nextId_;
}



