//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "utf.h"
// #include <boost/smart_ptr.hpp>

using namespace adportable;

// 16bit --> 8bit
std::basic_string<UTF8>
utf::to_utf8( const UTF16 * sourceStart )
{
   size_t utf16size = 0;
   const UTF16 * p = sourceStart;
   while ( *p++ )
      utf16size++;

   std::basic_string<UTF8> res( utf16size * 3 + 1, '\0' );
   const UTF16 * sourceEnd = sourceStart + utf16size;
   UTF8 * targetStart = &res[0];
   UTF8 * targetEnd = targetStart + utf16size * 3;
   if ( ConvertUTF16toUTF8( &sourceStart, sourceEnd, &targetStart, targetEnd, strictConversion ) != conversionOK )
       throw std::exception("ConvertUTF16toUTF8 failed");
   *targetStart = '\0';
   return res;
}

// 32bit --> 8bit
std::basic_string<UTF8>
utf::to_utf8( const UTF32 * sourceStart )
{
   size_t utf32size = 0;
   const UTF32 * p = sourceStart;
   while ( *p++ )
      utf32size++;

   std::basic_string<UTF8> res( utf32size * 4 + 1, '\0' );
   const UTF32 * sourceEnd = sourceStart + utf32size;
   UTF8 * targetStart = &res[0];
   UTF8 * targetEnd = targetStart + utf32size * 4;
   if ( ConvertUTF32toUTF8( &sourceStart, sourceEnd, &targetStart, targetEnd, strictConversion ) != conversionOK )
       throw std::exception("ConvertUTF32toUTF8 failed");
   *targetStart = '\0';
   return res;
}

//////////////////////////////////////////////
// 16bit --> 32bit
std::basic_string<UTF32>
utf::to_utf32( const UTF16 * sourceStart )
{
   size_t utf16size = 0;
   const UTF16 * p = sourceStart;
   while ( *p++ )
      utf16size++;
   std::basic_string< UTF32 > res( utf16size + 1, '\0' );
   const UTF16 * sourceEnd = sourceStart + utf16size;
   UTF32 * targetStart = &res[0];
   UTF32 * targetEnd = targetStart + utf16size;
   if ( ConvertUTF16toUTF32( &sourceStart, sourceEnd, &targetStart, targetEnd, strictConversion ) != conversionOK )
       throw std::exception("ConvertUTF16toUTF32 failed");
   *targetStart = 0;
   return res;
}

//////////////////////////////////////////////
// 8bit --> 32bit
std::basic_string<UTF32>
utf::to_utf32( const UTF8 * sourceStart )
{
   size_t utf8size = 0;
   const UTF8 * p = sourceStart;
   while ( *p++ )
      utf8size++;

   std::basic_string< UTF32 > res( utf8size + 1, '\0' );
   const UTF8 * sourceEnd = sourceStart + utf8size;
   UTF32 * targetStart = &res[0];
   UTF32 * targetEnd = targetStart + utf8size;
   if ( ConvertUTF8toUTF32( &sourceStart, sourceEnd, &targetStart, targetEnd, strictConversion ) != conversionOK )
       throw std::exception("ConvertUTF8toUTF32 failed");
   *targetStart = 0;
   return res;
}

//////////////////////////////////////////////
// 8bit --> 16bit
std::basic_string<UTF16>
utf::to_utf16( const UTF8 * sourceStart )
{
   size_t utf8size = 0;
   const UTF8 * p = sourceStart;
   while ( *p++ )
      utf8size++;

   std::basic_string< UTF16 > res( utf8size + 1, '\0' );
   const UTF8 * sourceEnd = sourceStart + utf8size;
   UTF16 * targetStart = &res[0];
   UTF16 * targetEnd = targetStart + utf8size;
   if ( ConvertUTF8toUTF16( &sourceStart, sourceEnd, &targetStart, targetEnd, strictConversion ) != conversionOK )
       throw std::exception("ConvertUTF8toUTF16 failed");
   *targetStart = 0;
   return res;
}

//////////////////////////////////////////////
// 32bit --> 16bit
std::basic_string<UTF16>
utf::to_utf16( const UTF32 * sourceStart )
{
   size_t utf32size = 0;
   const UTF32 * p = sourceStart;
   while ( *p++ )
      utf32size++;

   std::basic_string< UTF16 > res( utf32size * 3 + 1, '\0' );
   const UTF32 * sourceEnd = sourceStart + utf32size;
   UTF16 * targetStart = &res[0];
   UTF16 * targetEnd = targetStart + utf32size;
   if ( ConvertUTF32toUTF16( &sourceStart, sourceEnd, &targetStart, targetEnd, strictConversion ) != conversionOK )
       throw std::exception("ConvertUTF32toUTF16 failed");
   *targetStart = 0;
   return res;
}

