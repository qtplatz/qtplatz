/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "codecvt.hpp"
#include "../adportable/ConvertUTF.h"
#include <stdexcept>
#include <cassert>

namespace adportable_serializer {

    std::string
    to_utf8( const std::wstring& t )
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
            throw std::runtime_error( "ConvertUTF16toUTF8 failed" );
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
            throw std::runtime_error( "ConvertUTF16toUTF8 failed" );
        *targetStart = '\0';
        size_t size = std::distance(&target[0], reinterpret_cast<char *>(targetStart));
        target.resize( size );
#endif
        return target;
    }

    std::wstring
    to_wstring( const std::string& u8 )
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
            throw std::runtime_error( "ConvertUTF16toUTF8 failed" );
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
            throw std::runtime_error( "ConvertUTF16toUTF8 failed" );
        }
        *targetStart = 0;
        size_t size = std::distance(&target[0], reinterpret_cast<wchar_t *>(targetStart));
        target.resize( size );
        return target;
#endif
    }
}
