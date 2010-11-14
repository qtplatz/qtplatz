// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"
#include "visitor.h"
#include <string>

namespace adcontrols {

    class datafile_factory;
    class datafile;

    class ADCONTROLSSHARED_EXPORT datafileBroker : public Visitor {
    protected:
        ~datafileBroker();
        datafileBroker();
    public:
        static bool register_library( const std::wstring& sharedlib );
        static bool register_factory( datafile_factory *, const std::wstring& name );
        static datafile_factory* find( const std::wstring& name );
        //
        static datafile * open( const std::wstring& filename, bool readonly = false );
    };
    
}

