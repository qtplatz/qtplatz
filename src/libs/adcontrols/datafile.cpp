//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "datafile.h"
#include "datafilebroker.h"

using namespace adcontrols;

datafile*
datafile::open( const std::wstring& filename, bool readonly )
{
    return datafileBroker::open( filename, readonly );
}
