// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include <cctype>
#include <string>

namespace adportable {

    struct split_filename {

        // ex. RUN_0001 ==> '0001'
        template<typename char_type>
        static std::basic_string<char_type> trailer_number( const std::basic_string<char_type>& stem, int digits = 4 ) {
            std::string::size_type pos = stem.size();
            for ( auto it = stem.rbegin(); it != stem.rend() && std::isdigit( *it ) && digits--; ++it )
                --pos;
            return stem.substr( pos );
        }

        // ex. RUN_0001 ==> 'RUN_'
        template<typename char_type>
        static std::basic_string<char_type> prefix( const std::basic_string<char_type>& stem, int digits = 4 ) {
            std::string::size_type pos = stem.size();            
            for ( auto it = stem.rbegin(); it != stem.rend() && std::isdigit( *it ) && digits--; ++it )
                --pos;
            return stem.substr( 0, pos );            
        }

        template<typename char_type>
        static int trailer_number_int( const std::basic_string<char_type>& stem, int digits = 4 ) {
            unsigned int number( 0 ), order( 1 );
            for ( auto it = stem.rbegin(); it != stem.rend() && std::isdigit( *it ) && digits--; ++it ) {
                number += ( (*it) - char_type('0') ) * order;
                order *= 10;
            }
            return number;
        }
    };

}
