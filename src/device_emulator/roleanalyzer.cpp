//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "roleanalyzer.h"

RoleAnalyzer::~RoleAnalyzer()
{
}

RoleAnalyzer::RoleAnalyzer()
{
}

RoleAnalyzer::RoleAnalyzer( const RoleAnalyzer& t ) : device_state( t )
                                                    , setpts_(t.setpts_)
                                                    , actuals_(t.actuals_)  
{
}

////////////////////////////////////////////////

using namespace analyzer;

analyzer_data::analyzer_data()
{
    const size_t size = offsetof(analyzer_data, __tail__) - offsetof(analyzer_data, __head__);
    memset( &__head__, 0, size );
}

analyzer_data::analyzer_data( const analyzer_data& t ) 
{
    const size_t size = offsetof(analyzer_data, __tail__) - offsetof(analyzer_data, __head__);
    model = t.model;
    hardware_rev = t.hardware_rev;
    firmware_rev = t.firmware_rev;
    serailnumber = t.serailnumber;
    memcpy(&__head__, &t.__head__, size );
}
