// -*- C++ -*-
/**************************************************************************
 ** Code was modified from the deviantfan's answer in
 ** https://stackoverflow.com/questions/33165171/c-shiftjis-to-utf8-conversion
 **
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

#include "sjis2utf8.hpp"
#include "shiftjis.h"
#include <string>
#include <sstream>

namespace {

    const static std::tuple<
        std::pair< char, size_t >
        , std::pair< char, size_t >
        , std::pair< char, size_t > > __offs = { {0x8, 0x100}, {0x9, 0x1100}, {0xe, 0x2100} };

}

namespace adportable {

    // const static std::array< std::pair<char, size_t>, 3 > __offset = { {0x8, 0x100}, {0x9, 0x1100}, {0xe, 0x2100} };

    std::string
    sjis2utf8::operator()( const std::string& input ) const
    {
        std::basic_ostringstream<char> os;
        size_t indexInput{0};

        while ( indexInput < input.size() ){
            char arraySection = static_cast< uint8_t >( input[indexInput] ) >> 4;

            size_t arrayOffset =
                arraySection == std::get<0>(__offs).first ? std::get<0>(__offs).second
                : arraySection == std::get<1>(__offs).first ? std::get<1>(__offs).second
                : arraySection == std::get<2>(__offs).first ? std::get<2>(__offs).second
                : 0;

            //determining real array offset
            if ( arrayOffset )  {
                arrayOffset += (static_cast< uint8_t >(input[indexInput]) & 0xf) << 8;
                indexInput++;
                if ( indexInput >= input.size() )
                    break;
            }

            arrayOffset += static_cast< uint8_t >( input[ indexInput++ ] );
            arrayOffset <<= 1;

            uint16_t unicodeValue = ( shiftJIS_convTable [ arrayOffset ] << 8) | shiftJIS_convTable [ arrayOffset + 1 ];

            //converting to UTF8
            if ( unicodeValue < 0x80 ) {
                os << static_cast< uint8_t >(unicodeValue);
            } else if( unicodeValue < 0x800 )  {
                os << uint8_t( 0xc0 | (unicodeValue >> 6) );
                os << uint8_t( 0x80 | (unicodeValue & 0x3f) );
            } else {
                os << uint8_t( 0xe0 | (unicodeValue >> 12) );
                os << uint8_t( 0x80 | ((unicodeValue & 0xfff) >> 6 ));
                os << uint8_t( 0x80 | (unicodeValue & 0x3f) );
            }
        }
        return os.str();
    }
};
