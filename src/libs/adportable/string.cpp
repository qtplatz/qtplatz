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
    ::MultiByteToWideChar( CP_ACP, 0, source.c_str(), source.length(), &target[0], target.size() );
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
    ::WideCharToMultiByte( CP_ACP, 0, source.c_str(), source.length(), &target[0], target.size(), 0, 0 );
#else
    size_t n = wcstombs( &target[0], source.c_str(), target.size() );
    if ( n != size_t( -1 ) ) 
	target.resize(n);
#endif
    return target;
}

// std::wstring
// string::wstring( const u8string& u8str )
// {
// #if defined WIN32
//     return std::wstring( reinterpret_cast< const wchar_t *>( utf::to_utf16( u8str.c_str() ).c_str() ) );
// #else
//     return std::wstring( reinterpret_cast< const wchar_t *>( utf::to_utf32( u8str.c_str() ).c_str() ) );
// #endif
// }

// std::wstring
// string::wstring( const unsigned char * u8 )
// {
// #if defined WIN32
//     return std::wstring( reinterpret_cast< const wchar_t *>( utf::to_utf16( u8 ).c_str() ) );
// #else
//     return std::wstring( reinterpret_cast< const wchar_t *>( utf::to_utf32( u8 ).c_str() ) );
// #endif
// }

// //static
// u8string
// string::utf8( const wchar_t * p )
// {
// #if defined WIN32
//     return u8string( utf::to_utf8( reinterpret_cast<const UTF16 *>(p) ) );
// #elif defined __linux__ && defined __darwin__
//     return u8string( utf::to_utf8( reinterpret_cast<const UTF32 *>(p) ) );
// #else
//     return u8string( utf::to_utf8( reinterpret_cast<const UTF32 *>(p) ) );
// #endif
// }

// //static
// u8string
// string::utf8( const u16char_t * p )
// {
//     return u8string( utf::to_utf8( reinterpret_cast<const UTF16 *>(p) ) );
// }

// //static
// u8string
// string::utf8( const u32char_t * p )
// {
//     return u8string( utf::to_utf8( reinterpret_cast<const UTF32 *>(p) ) );
// }

// //static
// u16string
// string::utf16( const wchar_t * p )
// {
//     return u16string( reinterpret_cast<const UTF16 *>(p) ); // no conversion
// }

// //static
// u32string
// string::utf32( const wchar_t * p )
// {
//     return utf::to_utf32( reinterpret_cast<const UTF32 *>(p) );
// }

/////////////////////////////////////
// u8string::u8string()
// {
// }

// u8string::u8string( const std::wstring& t )
//   : std::basic_string<u8char_t>( string::utf8( t.c_str() ) )
// {
// }

// u8string::u8string( const std::basic_string<u8char_t>& t )
//   : std::basic_string<u8char_t>(t)
// {
// }

// u8string::u8string( const std::basic_string<u16char_t>& t )
//   : std::basic_string<u8char_t>( string::utf8( t.c_str() ) )
// {
// }

// u8string::u8string( const std::basic_string<u32char_t>& t )
//   : std::basic_string<u8char_t>( string::utf8( t.c_str() ) )
// {
// }

// /////////////////////////////////
// u16string::u16string()
// {
// }

// u16string::u16string( const std::basic_string<u16char_t>& t )
//   : std::basic_string<u16char_t>( t )
// {
// }

// //////////////////////////////////
// u32string::u32string( const std::basic_string<u32char_t>& t )
//   : std::basic_string<u32char_t>( t )
// {
// }

