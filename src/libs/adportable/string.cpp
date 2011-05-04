//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "string.hpp"
#include "utf.hpp"

#ifdef WIN32
# include <Windows.h>
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
#if defined WIN32
    size_t source_length = source.length();
    std::wstring target( source_length + 1, L'\0' );
    ::MultiByteToWideChar( CP_ACP, 0, source.c_str(), source.length(), &target[0], target.size() );
#else
    std::basic_string<UTF16> utf16 = utf::to_utf16( reinterpret_cast<const UTF8 *>(source.c_str()) );
    std::wstring target( utf16.length(), '\0' );
    std::copy( utf16.begin(), utf16.end(), target.begin() );
#endif
    return target;
}

// static
std::string
string::convert( const std::wstring& source )
{
#if defined WIN32
    std::string target( source.length() * 3 + 1, '\0' );
    ::WideCharToMultiByte( CP_ACP, 0, source.c_str(), source.length(), &target[0], target.size(), 0, 0 );
#else
    std::basic_string<UTF8> utf8 = utf::to_utf8( reinterpret_cast<const UTF16 *>(source.c_str()) );
    std::string target( utf8.length(), '\0' );
    std::copy( utf8.begin(), utf8.end(), target.begin() );
#endif
    return target;
}

//static
u8string
string::utf8( const wchar_t * p )
{
    return u8string( utf::to_utf8( reinterpret_cast<const UTF16 *>(p) ) );
}

//static
u8string
string::utf8( const u16char_t * p )
{
    return u8string( utf::to_utf8( reinterpret_cast<const UTF16 *>(p) ) );
}

//static
u8string
string::utf8( const u32char_t * p )
{
    return u8string( utf::to_utf8( reinterpret_cast<const UTF32 *>(p) ) );
}

//static
u16string
string::utf16( const wchar_t * p )
{
    return u16string( reinterpret_cast<const UTF16 *>(p) ); // no conversion
}

//static
u32string
string::utf32( const wchar_t * p )
{
    return utf::to_utf32( reinterpret_cast<const UTF16 *>(p) );
}

/////////////////////////////////////
u8string::u8string()
{
}

u8string::u8string( const std::basic_string<u8char_t>& t )
: std::basic_string<u8char_t>(t)
{
}

u8string::u8string( const std::basic_string<u16char_t>& t )
: std::basic_string<u8char_t>( string::utf8( t.c_str() ) )
{
}

u8string::u8string( const std::basic_string<u32char_t>& t )
: std::basic_string<u8char_t>( string::utf8( t.c_str() ) )
{
}

/////////////////////////////////
u16string::u16string()
{
}

u16string::u16string( const std::basic_string<u16char_t>& t )
: std::basic_string<u16char_t>( t )
{
}

//////////////////////////////////
u32string::u32string( const std::basic_string<u32char_t>& t )
: std::basic_string<u32char_t>( t )
{
}