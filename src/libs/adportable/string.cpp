//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

#include "string.hpp"
#include "utf.hpp"
#include <iostream>

#ifdef WIN32
# include <Windows.h>
#else
# include <cstdlib>
#endif

using namespace adportable;

string::string(void)
{
}

string::~string(void)
{
}

// static
std::wstring
string::convert( const std::string& source )
{
    size_t source_length = source.length();
    std::wstring target( source_length, L'\0' );
#if defined WIN32
    ::MultiByteToWideChar( CP_ACP, 0, source.c_str(), int( source.length() ), &target[0], int(target.size()) );
#else
    size_t n = mbstowcs( &target[0], source.c_str(), target.size() );
    if ( n != size_t(-1) )
		target.resize( n );
#endif
    return target;
}

// static
std::string
string::convert( const std::wstring& source )
{
    std::string target( source.length() * 3 + 1, '\0' );
#if defined WIN32
    ::WideCharToMultiByte( CP_ACP, 0, source.c_str(), int( source.length()), &target[0], int(target.size()), 0, 0 );
#else
    size_t n = wcstombs( &target[0], source.c_str(), target.size() );
    if ( n != size_t( -1 ) ) 
	target.resize(n);
#endif
    return target;
}


