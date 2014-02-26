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

#include <string>

namespace adportable {

    typedef std::string u8string;

    class string {
    public:
        string(void);
        ~string(void);

        static std::wstring convert( const std::string& t );
        static std::string convert( const std::wstring& t );
        //static std::wstring wstring( const u8string& );
        //static std::wstring wstring( const unsigned char * );

        //static u8string utf8( const wchar_t * );
        //static u8string utf8( const u16char_t * );
        //static u8string utf8( const u32char_t * );
        //static u16string utf16( const wchar_t * );
        //static u32string utf32( const wchar_t * );
    };
}
