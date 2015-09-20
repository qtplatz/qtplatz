/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <cinttypes>

namespace adportable {

    // default parameter is based on 12bit adc, 2's complement integer is expected.

    // --------  bitfs maybe 0x800, depend on design ---------
    
    template<typename T, T bitfs = 0x7ff, T signb = 0x800, T mask = 0xfff>
    struct ad_bipoler {

        static double world_value(T adval, double fs) {
            if ( adval & signb )
                return -double((~adval & mask) + 1) / bitfs * fs;
            else
                return double(adval & mask) / bitfs * fs;
        }

        static T digital_value(double val, double fs) {
            if ( val < 0 )
                return (static_cast<T>( (val * bitfs / fs) + 0.5 ) - 1) & mask;
            else
                return static_cast<T>( (val * bitfs / fs) + 0.5 );
        }
    };

    // --------- unipoler setup --------------------------

    template<typename T, T bitfs = 0xfff, T mask = 0xfff>
    struct ad_unipoler {

        static double world_value(T adval, double fs) {
            return double(adval & mask) * fs / bitfs;
        }

        static T digital_value(double val, double fs) {
            return static_cast<T>( (val * bitfs / fs) + 0.5 );
        }
    };

    typedef ad_bipoler<uint16_t,     0x7ff,    0x800,    0xfff> ad12bit_bipoler_t;
    typedef ad_bipoler<uint16_t,    0x7fff,   0x8000,   0xffff> ad16bit_bipoler_t;
    typedef ad_bipoler<uint32_t,   0x7ffff,  0x80000,  0xfffff> ad20bit_bipoler_t;
    typedef ad_bipoler<uint32_t,  0x7fffff, 0x800000, 0xffffff> ad24bit_bipoler_t;

    typedef ad_bipoler<uint16_t,     0x800,    0x800,    0xfff> ad12xbit_bipoler_t;
    typedef ad_bipoler<uint16_t,    0x8000,   0x8000,   0xffff> ad16xbit_bipoler_t;
    typedef ad_bipoler<uint32_t,   0x80000,  0x80000,  0xfffff> ad20xbit_bipoler_t;
    typedef ad_bipoler<uint32_t,  0x000000, 0x800000, 0xffffff> ad24xbit_bipoler_t;

    typedef ad_unipoler<uint16_t,     0xfff,    0xfff> ad12bit_unipoler_t;
    typedef ad_unipoler<uint16_t,    0xffff,   0xffff> ad16bit_unipoler_t;
    typedef ad_unipoler<uint32_t,   0xfffff,  0xfffff> ad20bit_unipoler_t;
    typedef ad_unipoler<uint32_t,  0xffffff, 0xffffff> ad24bit_unipoler_t;

}
