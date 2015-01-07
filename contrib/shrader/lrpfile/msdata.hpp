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

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace shrader {

    namespace detail { struct msdata; }

    class msdata {
        enum {
            record_type_code = 5
            , block_size = 256
        };
    public:
        ~msdata();
        msdata( std::istream& in, size_t fsize );
        inline operator bool () const { return loaded_; }

        inline static bool is_internal_reference( int16_t flags ) { return flags & 0x20 ? true : false; } // else internal
        inline static bool is_profile( int16_t flags ) { return flags & 0x80 ? true : false; }       // b7, else raw (time?)
        inline static bool is_SIM( int16_t flags ) { return flags & 0x40 ? true : false; }           // b6, else scan
        inline static bool is_mass_array( int16_t flags ) { return flags & 0x10 ? true : false; }    // b4 (mass|time)
        inline static bool is_SRM( int16_t flags ) { return (flags & 0x70) == 0x70 ? true : false; } // b0(mass),1(intrn),2(sim)
        inline static bool is_negative_ion( int16_t flags ) { return flags & 0x4000 ? true : false; }

        int16_t scan( size_t block = 0 ) const;
        int16_t flags( size_t block = 0 ) const;
        int16_t threshold( size_t block = 0 ) const;
        int16_t nions( size_t block = 0 ) const;
        int32_t xlow( size_t block = 0 ) const;   // mass * 65536 | time * 16
        int32_t xhigh( size_t block = 0 ) const;  // mass * 65536 | time * 16

        size_t size() const;
        std::pair< const int32_t *, size_t > intensities( size_t block ) const;
        std::pair< const std::pair< int32_t, int32_t >*, size_t > ions( size_t block ) const;

    private:
        std::vector< std::shared_ptr< detail::msdata > > data_;
        bool loaded_;
    };

}

