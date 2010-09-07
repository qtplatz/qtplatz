//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "description.h"
#include <acewrapper/timeval.h>

using namespace adcontrols;

Description::~Description()
{
}

Description::Description()
{
   acewrapper::gettimeofday(tv_sec_, tv_usec_);
}

Description::Description( const std::wstring& key, const std::wstring& text ) : key_(key), text_(text)
{
    acewrapper::gettimeofday(tv_sec_, tv_usec_);
}

Description::Description( const Description& t ) : tv_sec_(t.tv_sec_)
						                         , tv_usec_(t.tv_usec_)
												 , key_(t.key_)
												 , text_(t.text_)
												 , xml_(t.xml_)
{
}

