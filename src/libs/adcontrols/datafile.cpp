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
    datafile * file = datafileBroker::open( filename, readonly );
    if ( file ) {
        file->filename_ = filename;
        file->readonly_ = readonly;
    }
    return file;
}

void
datafile::close( datafile *& file )
{
    delete file;
    file = 0;
}

const std::wstring&
datafile::filename() const
{
    return filename_;
}

bool
datafile::readonly() const
{
    return readonly_;
}