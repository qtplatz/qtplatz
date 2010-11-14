// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"
#include <string>

namespace adcontrols {
    
    class Visitor;
    
    class ADCONTROLSSHARED_EXPORT datafile {
    public:
        datafile(void) {}
        virtual ~datafile(void) {}
        
        typedef datafile * (*factory_type)(void);
        
        virtual void accept( Visitor& ) = 0;
        virtual factory_type factory() = 0;
        
        static bool access( const std::wstring& filename );
        static datafile * open( const std::wstring& filename, bool readonly = false );
        static void close( datafile * );
    private:

    };

}

