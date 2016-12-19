/**************************************************************************
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this 
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

#include <cstdint>
#include <array>

namespace adportable {

    // 8bit (uint8_t) gray code -- binary conversion
    class graycode {
        
        static std::array< uint16_t, 256 > code_;
        
        graycode( const graycode& ) = delete;
        const graycode& operator = ( const graycode& ) = delete;
    public:
        
        graycode();
        
        uint16_t operator()( uint16_t code ) const {
            
            if ( code < code_.size() )
                return code_.at( code );
            
            return (-1);
        }
        
        static uint16_t to_binary( uint16_t gray );
        static uint16_t to_graycode( uint16_t binary );
    };
}

