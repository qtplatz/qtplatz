// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#pragma once

#include "adportable_global.h"
#include <string>

namespace adportable {

    class ADPORTABLESHARED_EXPORT utf;

    class utf {
    public:
        static std::string to_utf8( const std::wstring& /* utf16|utf32 */ );
        static std::wstring to_wstring( const std::string& /* utf8 */ );
        static std::wstring to_wstring( const unsigned char * /* utf8 */ );
        static bool validate( const std::string& );
        template< typename T> static std::string as_utf8( const T& );
        template< typename T> static std::wstring as_wide( const T& );
    };

    template<> std::string utf::as_utf8( const std::basic_string< char >& t );
    template<> std::string utf::as_utf8( const std::basic_string< wchar_t >& t );

    template<> std::wstring utf::as_wide( const std::basic_string< char >& t );
    template<> std::wstring utf::as_wide( const std::basic_string< wchar_t >& t );
}
