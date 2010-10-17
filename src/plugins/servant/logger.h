// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <sstream>

namespace servant {
    
    class Logger {
        std::wstring srcid_;
        std::wostringstream stream_;
    public:
        Logger( const std::wstring& srcid = L"Servant" );
        ~Logger(void);
        static void initialize();
        static void shutdown();
        void operator()( const std::wstring& );
        void operator()( const std::string& );
    };

}
