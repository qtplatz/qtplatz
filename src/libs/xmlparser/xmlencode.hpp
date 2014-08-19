// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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
#include <sstream>

namespace xmlparser {

    template<typename char_type> std::basic_string<char_type> cdata( const std::basic_string<char_type>& src ) {
        std::basic_ostringstream<char_type> dst;
        dst << "<![CDATA[" << src << "]]>";
        return dst.str();
    }

    template<typename char_type> std::basic_string<char_type> encode( const std::basic_string<char_type>& src ) {
        std::basic_ostringstream<char_type> dst;
        for ( auto c: src ) {
            switch(c) {
            case '&': dst << "&amp;"; break;
            case '<': dst << "&lt;"; break;
            case '>': dst << "&gt;"; break;
            case '"': dst << "&quot;"; break;
            case '\'': dst << "&apos;"; break;
            default:
                dst << c;
                break;
            }
        }
        return dst.str();
    };

}

