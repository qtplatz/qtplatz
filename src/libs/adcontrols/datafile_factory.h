// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"
#include <string>

namespace adcontrols {

    class datafile;
    
    class ADCONTROLSSHARED_EXPORT datafile_factory {
    public:
        datafile_factory(void) {}
        virtual ~datafile_factory(void) {}
        
        typedef datafile_factory * (*factory_type)(void);
        
        virtual const std::wstring& name() = 0;
        virtual bool access( const std::wstring& filename ) = 0;
        virtual datafile * open( const std::wstring& filename, bool readonly = false ) = 0;
        virtual void close( datafile * ) = 0;
    private:

    };
    
}

