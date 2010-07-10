// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <string>

class ACE_INET_Addr;

namespace acewrapper {

    template<class char_type> class basic_string {
        std::basic_string<char_type> t_;
    public:
        basic_string();
        basic_string( const ACE_INET_Addr& );
        operator const std::basic_string<char_type>& () { return t_; }
    };
    
    typedef basic_string<char> string;
    typedef basic_string<wchar_t> wstring;
  
}

