//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

#include "utf.hpp"
#include "ConvertUTF.h"
#include <exception>
#include "debug.hpp"
#include <cassert>

using namespace adportable;

namespace adportable {

    class exception : public std::exception {
    public:
	std::string text_;
	exception( const char * text ) : text_( text ) {}
	virtual ~exception() throw() {}
	const char * what() const throw() { return text_.c_str(); }
    };

}

std::string
utf::to_utf8( const std::wstring& t )
{
#if defined WIN32    
    assert( sizeof( wchar_t ) == sizeof( UTF16 ) );
    size_t utf16size = t.size();

    std::string target( utf16size * 3 + 1, '\0' );

    const UTF16 * sourceStart = reinterpret_cast<const UTF16 *>( t.c_str() );
    const UTF16 * sourceEnd   = sourceStart + utf16size;
    UTF8 * targetStart        = reinterpret_cast<UTF8 *>( &target[0] );
    UTF8 * targetEnd          = targetStart + utf16size * 3;
    if ( ConvertUTF16toUTF8( &sourceStart, sourceEnd, &targetStart, targetEnd, strictConversion ) != conversionOK ) {
        throw exception( "ConvertUTF16toUTF8 failed" );
    }
    *targetStart = '\0';
    size_t size = std::distance(&target[0], reinterpret_cast<char *>(targetStart));
    target.resize( size );
#else
    assert( sizeof( wchar_t ) == sizeof( UTF32 ) );
    const size_t utf32size = t.size();

    std::string target( utf32size * 4 + 1, '\0' );

    const UTF32 * sourceStart = reinterpret_cast<const UTF32 *>( t.c_str() );
    const UTF32 * sourceEnd   = sourceStart + utf32size;
    UTF8 * targetStart        = reinterpret_cast<UTF8 *>( &target[0] );
    UTF8 * targetEnd          = targetStart + utf32size * 4;
    if ( ConvertUTF32toUTF8( &sourceStart, sourceEnd, &targetStart, targetEnd, strictConversion ) != conversionOK )
        throw exception("ConvertUTF32toUTF8 failed");
    *targetStart = '\0';
    size_t size = std::distance(&target[0], reinterpret_cast<char *>(targetStart));
    target.resize( size );
#endif
    return target;
}

std::wstring
utf::to_wstring( const std::string& u8 )
{
#if defined WIN32    
    assert( sizeof( wchar_t ) == sizeof( UTF16 ) );
    size_t utf8size = u8.size();
    std::wstring target( utf8size + 1, '\0' );
    const UTF8 * sourceStart = reinterpret_cast<const UTF8 *>( u8.c_str() );
    const UTF8 * sourceEnd   = sourceStart + utf8size;
    UTF16 * targetStart      = reinterpret_cast<UTF16 *>( &target[0] );
    UTF16 * targetEnd        = targetStart + utf8size;
    ConversionResult success = ConvertUTF8toUTF16( &sourceStart, sourceEnd, &targetStart, targetEnd, strictConversion );
    if ( success != conversionOK ) {
        adportable::debug(__FILE__, __LINE__) << "ConvertUTF8toUTF16 failed. code=" << int(success);
        throw exception("ConvertUTF8toUTF16 failed");
    }
    *targetStart = 0;
    size_t size = std::distance(&target[0], reinterpret_cast<wchar_t *>(targetStart));
    target.resize( size );
    return target;
#else
    assert( sizeof( wchar_t ) == sizeof( UTF32 ) );
    size_t utf8size = u8.size();
    std::wstring target( utf8size + 1, '\0' );
    const UTF8 * sourceStart = reinterpret_cast<const UTF8 *>( u8.c_str() );
    const UTF8 * sourceEnd   = sourceStart + utf8size;
    UTF32 * targetStart      = reinterpret_cast<UTF32 *>( &target[0] );
    UTF32 * targetEnd        = targetStart + utf8size;
    ConversionResult success = ConvertUTF8toUTF32( &sourceStart, sourceEnd, &targetStart, targetEnd, strictConversion );
    if ( success != conversionOK ) {
        adportable::debug(__FILE__, __LINE__) << "ConvertUTF8toUTF32 failed. code=" << int(success);
        throw exception("ConvertUTF8toUTF32 failed");
    }
    *targetStart = 0;
    size_t size = std::distance(&target[0], reinterpret_cast<wchar_t *>(targetStart));
    target.resize( size );
    return target;
#endif
}

std::wstring
utf::to_wstring( const unsigned char * u8 )
{
    return to_wstring( std::string( reinterpret_cast< const char *>(u8) ) );
}

