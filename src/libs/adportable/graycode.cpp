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

#include "graycode.hpp"
#include <atomic>

namespace adportable {

    static std::atomic<bool> code_initialized_(false);
    
    std::array< uint16_t, 256 > graycode::code_ = { { 0, 0 } };
    
    graycode::graycode()
    {
        for (int i = 0; i < code_.size(); ++i)
            code_[i] = graycode::to_binary(i);
        code_initialized_ = true;
    }
    
    uint16_t 
    graycode::to_binary( uint16_t gray ) {
        
        uint16_t binary = gray & 128;
        
        binary |= (gray ^ (binary >> 1 )) & 64;
        binary |= (gray ^ (binary >> 1 )) & 32;
        binary |= (gray ^ (binary >> 1 )) & 16;
        binary |= (gray ^ (binary >> 1 )) & 8;
        binary |= (gray ^ (binary >> 1 )) & 4;
        binary |= (gray ^ (binary >> 1 )) & 2;
        binary |= (gray ^ (binary >> 1 )) & 1;
        
        return binary;
    };
    
    uint16_t
    graycode::to_graycode( uint16_t binary ) {
        return ( binary >> 1 ) ^ binary;
    }
}
