/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include <cstdint>
#include "u5303a_global.hpp"

class U5303ASHARED_EXPORT ppio {
#if defined _MSC_VER
    static const uint16_t BASEPORT = 0x378;
#else
    static constexpr uint16_t BASEPORT = 0x378;
#endif
    uint8_t data_;
    bool success_;
public:
    ppio();
    ~ppio();

    operator bool () const;
    uint8_t data() const;
    ppio& operator << ( const uint8_t d );
    ppio& operator |= ( const uint8_t d );
    ppio& operator &= ( const uint8_t d );
};

